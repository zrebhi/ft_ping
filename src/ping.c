#include "ft_ping.h"

/*
 * Constructs and transmits a single ICMP Echo Request.
 * Increments the sequence number upon execution.
 */
void send_ping(t_ping *ctx) {
    t_packet packet;
    
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
        ctx->stats.packets_transmitted++;
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

            update_stats(ctx, rtt);         
            
            printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", 
                    bytes_recv - ip_hdr_len, 
                    ctx->dest_ip, 
                    recv_seq, 
                    ip_hdr->ttl, 
                    rtt);
        }
    } else {
        if (ctx->is_verbose) {
            handle_icmp_error(ctx, icmp_hdr, &sender, bytes_recv, ip_hdr_len);
        }
    }
}