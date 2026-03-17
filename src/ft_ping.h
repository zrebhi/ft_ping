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

/* Networking headers */
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <signal.h>

# define PING_DATA_SIZE 56

/* Central state structure for ft_ping */
typedef struct s_ping {
    bool    is_help;
    bool    is_verbose;
    char    *target_host;
    
    // Network Resolution Fields
    struct sockaddr_in dest_addr;           // The binary address for the socket
    char               dest_ip[INET_ADDRSTRLEN]; // The string IP (e.g., "8.8.8.8")

    int     sockfd;                      // Socket file descriptor for sending ICMP packets

    // Packet Tracking
    pid_t    pid;        // Process ID to identify our packets
    uint16_t sequence;   // Tracks the ICMP sequence number
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

#endif
