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

# define PING_DATA_SIZE 56

# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sysexits.h>
# include <errno.h>

/* Networking headers */
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/in.h>

/* Central state structure for ft_ping */
typedef struct s_ping {
    bool    is_help;
    bool    is_verbose;
    char    *target_host;
    
    // Network Resolution Fields
    struct sockaddr_in dest_addr;           // The binary address for the socket
    char               dest_ip[INET_ADDRSTRLEN]; // The string IP (e.g., "8.8.8.8")

    int     sockfd;                      // Socket file descriptor for sending ICMP packets
} t_ping;

/* Function prototypes */
void print_usage(void);
int  parse_args(int argc, char **argv, t_ping *ctx);
int  resolve_dns(t_ping *ctx);
int  create_socket(t_ping *ctx);

#endif
