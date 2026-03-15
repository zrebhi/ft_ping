#include "ft_ping.h"

int main(int argc, char **argv) {
    t_ping ping_ctx = {0}; // Zero-initialize context
    int status;

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

    // Craft the packet in memory and send it out
    t_packet packet;
    
    ping_ctx.pid = getpid() & 0xFFFF; // Cast to 16-bit to fit the ICMP header
    ping_ctx.sequence = 0;

    craft_icmp_packet(&ping_ctx, &packet);

    ssize_t bytes_sent = sendto(ping_ctx.sockfd, 
                                &packet, 
                                sizeof(packet), 
                                0, 
                                (struct sockaddr *)&ping_ctx.dest_addr, 
                                sizeof(ping_ctx.dest_addr));

    if (bytes_sent < 0) {
        fprintf(stderr, "ping: sendto: %s\n", strerror(errno));
        return 1;
    }

    printf("Fired %zd bytes into the network.\n", bytes_sent);

    return EX_OK;
}
