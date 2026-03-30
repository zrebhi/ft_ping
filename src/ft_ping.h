#ifndef FT_PING_H
# define FT_PING_H

/* * Feature Test Macro: 
 * Unlocks POSIX and GNU extensions in the C standard library. 
 * We need this to expose hidden networking definitions (like NI_MAXHOST) 
 * that are turned off by default in strict C compilation.
 */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif


# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sysexits.h>
# include <errno.h>
# include <unistd.h>
# include <signal.h>
# include <sys/time.h>
# include <math.h>

/* Networking headers */
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>

# define PING_DATA_SIZE 56

/* Structure to track all our statistics for the final output */
typedef struct s_stats {
    size_t  packets_transmitted;
    size_t  packets_received;
    double  rtt_min;
    double  rtt_max;
    double  rtt_sum;
    double  rtt_sum_sq; // Sum of squares, needed for standard deviation
    struct  timeval start_time;
    struct  timeval end_time;
} t_stats;

/* Central state structure for ft_ping */
typedef struct s_ping {
    bool    is_help;
    bool    is_verbose;
    char    *target_host;
    
    // Network Resolution Fields
    struct sockaddr_in dest_addr;
    char               dest_ip[INET_ADDRSTRLEN];

    int     sockfd;

    // Packet Tracking
    pid_t    pid;
    uint16_t sequence;

    char    recv_buf[1024];
    
    t_stats stats;
} t_ping;

/* Packet Structure: Header (8 bytes) + Payload (56 bytes) = 64 bytes total */
typedef struct s_packet {
    struct icmphdr hdr;
    char           msg[PING_DATA_SIZE];
} t_packet;

/* Function prototypes */
void print_usage(void);
int  parse_args(int argc, char **argv, t_ping *ctx);
int  resolve_dns(t_ping *ctx);
int  create_socket(t_ping *ctx);
uint16_t calculate_checksum(uint16_t *data, size_t length);
void     craft_icmp_packet(t_ping *ctx, t_packet *pkt);
void update_stats(t_ping *ctx, double rtt);
void print_stats(t_ping *ctx);
void handle_icmp_error(t_ping *ctx, struct icmphdr *icmp_hdr, struct sockaddr_in *sender, ssize_t bytes_recv, size_t ip_hdr_len);
void print_ip_hdr_dump(struct iphdr *unaligned_ip, struct icmphdr *unaligned_icmp);
void send_ping(t_ping *ctx);
void receive_ping(t_ping *ctx);

#endif
