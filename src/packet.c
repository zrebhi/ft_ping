#include "ft_ping.h"

/*
 * RFC 1071 Internet Checksum
 * Chops the packet into 16-bit chunks, sums them up, and flips the bits.
 * This proves to the receiving router that the packet wasn't corrupted in transit.
 */
uint16_t calculate_checksum(uint16_t *data, size_t length) {
    uint32_t sum = 0;

    // Add up 16-bit words
    while (length > 1) {
        sum += *data++;
        length -= 2;
    }

    // Add leftover byte, if any
    if (length == 1) {
        sum += *(uint8_t *)data;
    }

    // Fold 32-bit sum into 16 bits
    sum = (sum >> 16) + (sum & 0xFFFF); // Adds the top 16 bits to the bottom 16 bits
    sum += (sum >> 16); // In case the previous addition caused an overflow, add again

    // Return the bitwise NOT
    return (uint16_t)(~sum);
}

/*
 * Assembles the ICMP Echo Request packet in memory.
 */
void craft_icmp_packet(t_ping *ctx, t_packet *pkt) {
    memset(pkt, 0, sizeof(t_packet));

    pkt->hdr.type = ICMP_ECHO;  // 8 (Ping Request)
    pkt->hdr.code = 0;          // 0 for Echo Request
    
    /* Use htons() to ensure correct byte order for the network */
    pkt->hdr.un.echo.id = htons(ctx->pid);
    pkt->hdr.un.echo.sequence = htons(ctx->sequence);

    /* Add timestamp at the beginning of our payload for round-trip time calculation */
    struct timeval tv_send;
    gettimeofday(&tv_send, NULL);

    memcpy(pkt->msg, &tv_send, sizeof(tv_send));

    for (size_t i = sizeof(tv_send); i < PING_DATA_SIZE - 1; i++) {
        pkt->msg[i] = 'a' + ((i - sizeof(tv_send)) % 26);
    }

    pkt->hdr.checksum = 0; // Must be zero BEFORE calculating checksum
    pkt->hdr.checksum = calculate_checksum((uint16_t *)pkt, sizeof(t_packet));
}
