#include "../ft_ping.h"

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

int main(int argc, char **argv) {
    t_ping ping_ctx = {0};
    int status;

    /* Force stdout to be line-buffered even when redirected. 
     * This ensures our test scripts capture the output before 
     * the process is killed by 'timeout'. 
     */
    setlinebuf(stdout);

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

    /* Register the alarm signal handler using sigaction */
    struct sigaction sa = {0}; 
    
    sa.sa_handler = alarm_handler;
    /* * Ensure SA_RESTART is disabled preventing the OS 
     * from automatically restarting system calls interrupted by a signal */
    sa.sa_flags = 0;
    
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        fprintf(stderr, "ping: sigaction: %s\n", strerror(errno));
        return 1;
    }

    /* Register the Ctrl+C signal handler */
    signal(SIGINT, int_handler);

    /* Main execution loop */
    while (g_action != 2) {
        if (g_action == 1) {
            send_ping(&ping_ctx);
            g_action = 0;
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
