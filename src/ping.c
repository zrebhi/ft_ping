#include "ft_ping.h"

/*
 * Constructs and transmits a single ICMP Echo Request.
 * Increments the sequence number upon execution.
 */
void send_ping(t_ping *ctx) {
    if (!ctx)
        return;

    t_packet packet;
    craft_icmp_packet(ctx, &packet);

    int send_flags = 0;
    if (ctx->is_ignore_routing) {
        send_flags = MSG_DONTROUTE;
    }
    
    ssize_t bytes_sent = sendto(ctx->sockfd, 
        &packet, 
        sizeof(packet), 
        send_flags, 
        (struct sockaddr *)&ctx->dest_addr, 
        sizeof(ctx->dest_addr));
        
        if (bytes_sent < 0) {
            fprintf(stderr, "ping: sending packet: %s\n", strerror(errno));
        } else {
        ctx->stats.packets_transmitted++;
    }
}

/*
 * Processes a validated ICMP Echo Reply.
 * Extracts the timestamp, calculates RTT, updates statistics, and prints the result.
 */
static void handle_echo_reply(t_ping *ctx, struct iphdr *ip_hdr, 
                              struct icmphdr *icmp_hdr, ssize_t bytes_recv, 
                              size_t ip_hdr_len) 
{
    uint16_t recv_id = ntohs(icmp_hdr->un.echo.id);
    uint16_t recv_seq = ntohs(icmp_hdr->un.echo.sequence);

    /* Only process our own ping replies */
    if (recv_id == ctx->pid) {
        struct timeval tv_recv, tv_send;
        
        gettimeofday(&tv_recv, NULL);
        
        /* Extract our send timestamp from the payload */
        char *payload = (char *)icmp_hdr + sizeof(struct icmphdr);
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
}

/*
 * Validates the raw received buffer, ensuring IP, ICMP, and payload boundaries are intact.
 * Acts as the absolute security gate before dispatching to handlers.
 */
static void process_packet(t_ping *ctx, ssize_t bytes_recv, struct sockaddr_in *sender) 
{
    /* Validate minimum IPv4 Header size */
    if (bytes_recv < (ssize_t)sizeof(struct iphdr)) return;

    struct iphdr *ip_hdr = (struct iphdr *)ctx->recv_buf;
    
    /* Validate IHL to prevent arithmetic underflow */
    if (ip_hdr->ihl < 5) return; 

    size_t ip_hdr_len = ip_hdr->ihl * 4;

    /* Validate enough bytes for IP Header + ICMP Header */
    if (bytes_recv < (ssize_t)(ip_hdr_len + sizeof(struct icmphdr))) return;

    struct icmphdr *icmp_hdr = (struct icmphdr *)(ctx->recv_buf + ip_hdr_len);

    /* Dispatch based on ICMP Type with type-specific payload validation */
    if (icmp_hdr->type == ICMP_ECHOREPLY) {
        
        /* Validate payload size specifically for Echo Replies */
        if (bytes_recv < (ssize_t)(ip_hdr_len + sizeof(struct icmphdr) + sizeof(struct timeval))) {
            return; 
        }
        handle_echo_reply(ctx, ip_hdr, icmp_hdr, bytes_recv, ip_hdr_len);
    } else {
        handle_icmp_error(ctx, icmp_hdr, sender, bytes_recv, ip_hdr_len);
    }
}

/*
 * Blocks and waits for incoming packets from the raw socket.
 * Handles system call interruptions (EINTR) for timeouts.
 */
void receive_ping(t_ping *ctx) 
{
    if (!ctx)
        return;

    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    ssize_t bytes_recv = recvfrom(ctx->sockfd, 
                                  ctx->recv_buf, 
                                  sizeof(ctx->recv_buf), 
                                  0, 
                                  (struct sockaddr *)&sender, 
                                  &sender_len);

    if (bytes_recv < 0) {
        /* EINTR means our SIGALRM fired while waiting (timeout) */
        if (errno == EINTR) {
            return;
        }
        fprintf(stderr, "ping: recvfrom: %s\n", strerror(errno));
        return;
    }

    /* Pass the raw bytes to our validator and router */
    process_packet(ctx, bytes_recv, &sender);
}
