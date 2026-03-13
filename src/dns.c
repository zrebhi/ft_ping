#include "ft_ping.h"

/*
 * Translates a human-readable hostname (e.g., "google.com") into a 
 * machine-readable IPv4 network structure. 
 * Restricts the search to AF_INET (IPv4) and SOCK_RAW (ICMP).
 * * Returns: EX_OK (0) on success, or 1 if the host cannot be found
 * (matching the inetutils-2.0 exit code).
 */
int resolve_dns(t_ping *ctx) {
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    if (getaddrinfo(ctx->target_host, NULL, &hints, &result) != 0) {
        fprintf(stderr, "ping: unknown host\n");
        return 1; 
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)result->ai_addr;
    
    memcpy(&ctx->dest_addr, ipv4, sizeof(struct sockaddr_in));
    inet_ntop(AF_INET, &(ipv4->sin_addr), ctx->dest_ip, INET_ADDRSTRLEN);

    freeaddrinfo(result);
    
    return EX_OK;
}
