#include "ft_ping.h"

/*
 * ========================================================================
 * ICMP ERROR PACKET MEMORY LAYOUT
 * ========================================================================
 * When a router drops a packet (e.g., TTL exceeded), it replies with an 
 * ICMP error message that wraps the original failed packet's headers.
 * We must use pointer arithmetic to navigate this nested structure safely.
 * +------------------------------------+
 * | Outer IP Header                    | <- Skipped: (bytes_recv - ip_hdr_len)
 * | (Variable, usually 20 bytes)       |    Created by the responding router.
 * +------------------------------------+ <=== START OF icmp_len
 * | ICMP Error Header                  | <- 8 bytes. Contains Type (e.g., 11)
 * | (Exactly 8 bytes)                  |    and Code.
 * +------------------------------------+
 * | Original IP Header                 | <- Variable length (orig_ip_len). 
 * | (Usually 20 bytes)                 |    The exact header we originally sent.
 * +------------------------------------+
 * | Original ICMP Header               | <- 8 bytes. We must reach this to 
 * | (safely extract the process ID)    |    
 * +------------------------------------+
 * | Original ICMP Data                 | <- Variable. Payload of original ping.
 * +------------------------------------+
 * ========================================================================
 */

/*
 * Extracts the original ICMP header from an ICMP error packet.
 * Validates all lengths and headers to prevent buffer overflows or segfaults.
 * Returns a pointer to the original ICMP header, or NULL if the packet is invalid.
 */
static struct icmphdr *get_orig_icmp_header(struct icmphdr *icmp_hdr, ssize_t bytes_recv, size_t ip_hdr_len) {
    /* Prevent subtraction underflow if packet is severely truncated */
    if (bytes_recv < (ssize_t)ip_hdr_len) {
        return NULL;
    }

    ssize_t icmp_len = bytes_recv - ip_hdr_len;

    /* Ensure packet contains an ICMP error header (8 bytes) and a basic nested IPv4 header (20 bytes) */
    if (icmp_len < 8 + (ssize_t)sizeof(struct iphdr)) {
        return NULL; 
    }

    struct iphdr orig_ip;
    /* Skip the 8 bytes of the ICMP error header to find our original IP header */
    memcpy(&orig_ip, (char *)icmp_hdr + 8, sizeof(struct iphdr));
    size_t orig_ip_len = orig_ip.ihl * 4;

    /* Validate the nested IPv4 header length */
    if (orig_ip_len < sizeof(struct iphdr)) {
        return NULL;
    }

    /* Ensure packet contains the full nested IP header and at least 8 bytes of the nested ICMP header (to read the ID) */
    if (icmp_len < 8 + (ssize_t)orig_ip_len + 8) {
        return NULL;
    }

    /* Calculate the exact start of the original ICMP header */
    void *raw_orig_icmp = (char *)icmp_hdr + 8 + orig_ip_len;
    struct icmphdr safe_orig_icmp;
    memcpy(&safe_orig_icmp, raw_orig_icmp, 8);

    /* Ensure the original packet was actually an ECHO request 
     * before we trust the layout of the un.echo union */
    if (safe_orig_icmp.type != ICMP_ECHO) {
        return NULL;
    }

    return (struct icmphdr *)raw_orig_icmp;
}

/*
 * Extracts the router's IP address and prints the high-level ICMP error message.
 * Note: Reverse DNS resolution is explicitly omitted here to comply with 
 * the project requirement forbidding DNS resolution during packet return.
 */
static void print_icmp_error_msg(struct icmphdr *icmp_hdr, struct sockaddr_in *sender, ssize_t error_payload_len) {
    char router_ip[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &sender->sin_addr, router_ip, sizeof(router_ip));

    printf("%zd bytes from %s: ", error_payload_len, router_ip);

    if (icmp_hdr->type == ICMP_TIME_EXCEEDED) {
        printf("Time to live exceeded\n");
    } else {
        printf("ICMP Type %d\n", icmp_hdr->type); /* Fallback for Destination Unreachable, etc. */
    }
}


/* Helper to print the verbose IP Header Dump exactly like inetutils-2.0 */
static void print_ip_hdr_dump(struct iphdr *unaligned_ip, struct icmphdr *unaligned_icmp) {
    /* SAFE READ: Copy headers to aligned stack memory to prevent SIGBUS crashes */
   struct iphdr ip;
   memcpy(&ip, unaligned_ip, sizeof(struct iphdr));
   
   /* Now it is perfectly safe to cast to a 16-bit integer pointer */
   uint16_t *p = (uint16_t *)&ip;
   
   char src_ip[INET_ADDRSTRLEN];
   char dst_ip[INET_ADDRSTRLEN];
   
   inet_ntop(AF_INET, &ip.saddr, src_ip, sizeof(src_ip));
   inet_ntop(AF_INET, &ip.daddr, dst_ip, sizeof(dst_ip));
   
   printf("IP Hdr Dump:\n");
   /* Print 20 bytes of IP header as 16-bit hex words */
   printf(" %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x \n",
    ntohs(p[0]), ntohs(p[1]), ntohs(p[2]), ntohs(p[3]), ntohs(p[4]),
           ntohs(p[5]), ntohs(p[6]), ntohs(p[7]), ntohs(p[8]), ntohs(p[9]));
           
           printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src\tDst\tData\n");
           
           uint16_t frag = ntohs(ip.frag_off);
           
           printf(" %x  %x  %02x %04x %04x   %x %04x  %02x  %02x %04x %s  %s \n",
            ip.version,
            ip.ihl,
            ip.tos,
           ntohs(ip.tot_len),
           ntohs(ip.id),
           frag >> 13,       
           frag & 0x1FFF,    
           ip.ttl,
           ip.protocol,
           ntohs(ip.check),
           src_ip,
           dst_ip);
           
           /* Protect against corrupted tot_len causing subtraction underflow */
           uint16_t total_len = ntohs(ip.tot_len);
           uint16_t header_len = ip.ihl * 4;
           int icmp_payload_size = (total_len >= header_len) ? (total_len - header_len) : 0;
           
           /* SAFE READ: Copy headers to aligned stack memory to prevent SIGBUS crashes */
           struct icmphdr icmp;
           memcpy(&icmp, unaligned_icmp, 8);
           
           printf("ICMP: type %d, code %d, size %d, id 0x%04x, seq 0x%04x\n",
           icmp.type,
           icmp.code,
           icmp_payload_size,
           ntohs(icmp.un.echo.id),
           ntohs(icmp.un.echo.sequence));
}

/*
* Orchestrator: Parses ICMP error messages (like Time Exceeded) when in verbose mode.
* Extracts the original IP/ICMP headers to verify the error belongs to our process.
*/
void handle_icmp_error(t_ping *ctx, struct icmphdr *icmp_hdr, struct sockaddr_in *sender, ssize_t bytes_recv, size_t ip_hdr_len) {
    if (!ctx || !icmp_hdr || !sender) {
        return;
    }

    struct icmphdr *orig_icmp = get_orig_icmp_header(icmp_hdr, bytes_recv, ip_hdr_len);
    
    if (!orig_icmp) {
        return; /* Packet was invalid, truncated, or not an ECHO response */
    }

    /* SAFE READ: Copy headers to aligned stack memory to prevent SIGBUS crashes */
    struct icmphdr safe_icmp;
    memcpy(&safe_icmp, orig_icmp, 8); /* 8 bytes is enough to capture type, code, checksum, and ID */

    /* Verify this error was generated in response to OUR process */
    if (ntohs(safe_icmp.un.echo.id) == ctx->pid) {
        /* We can safely cast orig_ip here because get_orig_icmp_header already validated the bounds */
        struct iphdr *orig_ip = (struct iphdr *)((char *)icmp_hdr + 8);
        
        print_icmp_error_msg(icmp_hdr, sender, bytes_recv - ip_hdr_len);
        print_ip_hdr_dump(orig_ip, orig_icmp);
    }
}
