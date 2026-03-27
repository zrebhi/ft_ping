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

/*
 * Blocks and waits for incoming packets from the raw socket.
 * If no packet arrives, it relies on SIGALRM to interrupt the system call (EINTR),
 * preventing infinite hangs on dropped packets.
 * Extracts the IP header to find the ICMP payload.
 */
void receive_ping(t_ping *ctx) {
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    ssize_t bytes_recv = recvfrom(ctx->sockfd, 
                                  ctx->recv_buf, 
                                  sizeof(ctx->recv_buf), 
                                  0, 
                                  (struct sockaddr *)&sender, 
                                  &sender_len);

    if (bytes_recv < 0) {
        /* * EINTR means our 1-second SIGALRM fired while waiting.
         * This acts as our automatic timeout for lost/dropped packets.
         * We simply return to the main loop to fire the next ping.
         */
        if (errno == EINTR) {
            return;
        }
        fprintf(stderr, "ping: recvfrom: %s\n", strerror(errno));
        return;
    }

    struct iphdr *ip_hdr = (struct iphdr *)ctx->recv_buf; 
    /* ihl is given in 4-byte words so we multiply by 4 to get the length in bytes */
    size_t ip_hdr_len = ip_hdr->ihl * 4;
    /* Extract the ICMP Header by offsetting the buffer pointer */
    struct icmphdr *icmp_hdr = (struct icmphdr *)(ctx->recv_buf + ip_hdr_len);

    if (icmp_hdr->type == ICMP_ECHOREPLY) {
        /* Convert ID and Sequence from network byte order back to host byte order */
        uint16_t recv_id = ntohs(icmp_hdr->un.echo.id);
        uint16_t recv_seq = ntohs(icmp_hdr->un.echo.sequence);

        if (recv_id == ctx->pid) {
            struct timeval tv_recv, tv_send;
            
            /* Grab the exact time we received the packet */
            gettimeofday(&tv_recv, NULL);
            
            /* Extract our send timestamp from the payload (skipping the 8-byte ICMP header) */
            char *payload = (char *)icmp_hdr + sizeof(struct icmphdr);
            /* We added the timestamp at the beginning of the payload in craft_icmp_packet */
            memcpy(&tv_send, payload, sizeof(tv_send));
            
            /* Calculate RTT in milliseconds */
            double rtt = (tv_recv.tv_sec - tv_send.tv_sec) * 1000.0 + 
                         (tv_recv.tv_usec - tv_send.tv_usec) / 1000.0;
                         
            printf("[DEBUG] Valid Reply! seq=%d, id=%d, time=%.3f ms\n", recv_seq, recv_id, rtt);
        }
    } else {
        /* We ignore other ICMP types for now (like Destination Unreachable)
         * unless running in verbose mode, which we will handle later. */
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

    /* Main execution loop */
    while (1) {
        if (g_send_packet) {
            send_ping(&ping_ctx);
            g_send_packet = 0;
            alarm(1);
        }
        /* * Blocks here (0% CPU) until an ICMP packet arrives or the 1-second alarm fires.
         * This prevents busy-waiting and ensures RTT calculations remain highly accurate.
         */
        receive_ping(&ping_ctx);
    }

    return EX_OK;
}
