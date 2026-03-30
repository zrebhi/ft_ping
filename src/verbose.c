#include "ft_ping.h"

/* Helper to print the verbose IP Header Dump exactly like inetutils-2.0 */
void print_ip_hdr_dump(struct iphdr *unaligned_ip, struct icmphdr *unaligned_icmp) {
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
