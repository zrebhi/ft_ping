#include "ft_ping.h"

/*
 * Requests a raw socket from the kernel to transmit ICMP packets.
 * Returns: EX_OK (0) on success, or 1 on failure.
 */
int create_socket(t_ping *ctx) {
    ctx->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    if (ctx->sockfd < 0) {
        fprintf(stderr, "ping: socket: %s\n", strerror(errno));
        return 1;
    }

    if (getenv("FT_PING_FORCE_TTL")) {
        int ttl = 1;
        setsockopt(ctx->sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
    }

    return EX_OK;
}
