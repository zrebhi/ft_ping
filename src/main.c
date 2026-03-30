#include "ft_ping.h"

/* Global state machine modified by the signal handler.
 * 0 = IDLE, 1 = SEND_PACKET, 2 = STOP_LOOP
 * volatile ensures the compiler does not optimize it away.
 * sig_atomic_t ensures read/write operations are atomic and thread-safe.
 */
volatile sig_atomic_t g_action = 1;

/*
 * Catches the SIGALRM signal and sets the flag to trigger the next packet.
 * System calls and I/O are avoided here to maintain async-signal safety.
 */
void alarm_handler(int signum) {
    (void)signum;
    /* Only set to SEND if we haven't already been told to STOP */
    if (g_action != 2) {
        g_action = 1;
    }
}

/* Catches Ctrl+C (SIGINT) and gracefully stops the main loop */
void int_handler(int signum) {
    (void)signum;
    g_action = 2;
}

/*
 * Initializes and registers process-wide signal handlers.
 * Returns: 0 on success, or 1 on failure.
 */
int setup_signals(void) {
    /* Register the alarm signal handler */
    struct sigaction sa = {0}; 

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);   // Block SIGINT while in alarm_handler
    sigaddset(&sa.sa_mask, SIGTERM);  // Block SIGTERM while in alarm_handler

    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0; // Ensures no SA_RESTART

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        fprintf(stderr, "ping: sigaction: %s\n", strerror(errno));
        return 1;
    }

    /* Register the Ctrl+C signal handler */
    struct sigaction sa_int = {0}; 

    sigemptyset(&sa_int.sa_mask);
    sigaddset(&sa_int.sa_mask, SIGALRM); // Block SIGALRM while in int_handler

    sa_int.sa_handler = int_handler;
    sa_int.sa_flags = 0; // Ensures no SA_RESTART

    if (sigaction(SIGINT, &sa_int, NULL) == -1 || sigaction(SIGTERM, &sa_int, NULL) == -1) {
        fprintf(stderr, "ping: sigaction: %s\n", strerror(errno));
        return 1;
    }

    return 0; // Success
}

int main(int argc, char **argv) {
    t_ping ping_ctx = {0};
    int status;

    /* Force stdout to be line-buffered even when redirected. 
     * This ensures our test scripts capture the output before 
     * the process is killed by 'timeout'. 
     */
    setlinebuf(stdout);

    /* Shield the process immediately before network resolution */
    if (setup_signals() != 0) {
        return 1;
    }

    status = parse_args(argc, argv, &ping_ctx);
    if (status != EX_OK || ping_ctx.is_help) {
        return status;
    }

    status = resolve_dns(&ping_ctx);
    if (status != 0) {
        return status;
    }

    status = create_socket(&ping_ctx);
    if (status != 0) {
        return status;
    }

    /* Initialize Process ID for packet tracking */
    ping_ctx.pid = getpid() & 0xFFFF;

    if (ping_ctx.is_verbose) {
        printf("PING %s (%s): %d data bytes, id 0x%04x = %d\n", 
                ping_ctx.target_host, ping_ctx.dest_ip, PING_DATA_SIZE, 
                ping_ctx.pid, ping_ctx.pid);
    } else {
        printf("PING %s (%s): %d data bytes\n", 
                ping_ctx.target_host, ping_ctx.dest_ip, PING_DATA_SIZE);
    }

    /* Main execution loop */
    while (g_action != 2) {
        if (g_action == 1) {
            g_action = 0;
            send_ping(&ping_ctx);
            alarm(1);
        }
        /* * Blocks here (0% CPU) until an ICMP packet arrives or the 1-second alarm fires.
         * This prevents busy-waiting and ensures RTT calculations remain highly accurate.
         */
        receive_ping(&ping_ctx);
    }

    print_stats(&ping_ctx);

    if (ping_ctx.sockfd > 0) {
        close(ping_ctx.sockfd);
    }

    /* If we transmitted packets but received none, exit with error code 1 */
    if (ping_ctx.stats.packets_transmitted > 0 && ping_ctx.stats.packets_received == 0) {
        return 1;
    }

    return EX_OK;
}
