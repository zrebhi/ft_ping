#include "ft_ping.h"

/* Global flag modified by the signal handler.
 * volatile ensures the compiler does not optimize it away.
 * sig_atomic_t ensures read/write operations are atomic and thread-safe.
 */
volatile sig_atomic_t g_send_packet = 1;

/*
 * Catches the SIGALRM signal and sets the flag to trigger the next packet.
 * System calls and I/O are avoided here to maintain async-signal safety.
 */
void alarm_handler(int signum) {
    (void)signum;
    g_send_packet = 1;
}

/*
 * Constructs and transmits a single ICMP Echo Request.
 * Increments the sequence number upon execution.
 */
void send_ping(t_ping *ctx) {
    t_packet packet;
    
    ctx->pid = getpid() & 0xFFFF;
    ctx->sequence++;

    craft_icmp_packet(ctx, &packet);

    ssize_t bytes_sent = sendto(ctx->sockfd, 
                                &packet, 
                                sizeof(packet), 
                                0, 
                                (struct sockaddr *)&ctx->dest_addr, 
                                sizeof(ctx->dest_addr));

    if (bytes_sent < 0) {
        fprintf(stderr, "ping: sendto: %s\n", strerror(errno));
    } else {
        printf("[DEBUG] Success! Fired %zd bytes into the network. (seq=%d)\n", bytes_sent, ctx->sequence);
    }
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

    printf("PING %s (%s): %d data bytes\n", 
            ping_ctx.target_host, 
            ping_ctx.dest_ip, 
            PING_DATA_SIZE);

    /* Register the alarm signal handler */
    signal(SIGALRM, alarm_handler);

    /* Main execution loop */
    while (1) {
        if (g_send_packet) {
            send_ping(&ping_ctx);
            g_send_packet = 0;
            alarm(1);
        }
        /* Suspend execution until a signal is caught, preventing 100% CPU usage */
        pause(); 
    }

    return EX_OK;
}
