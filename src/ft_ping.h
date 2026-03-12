#ifndef FT_PING_H
# define FT_PING_H

# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sysexits.h>

/* Central state structure for ft_ping */
typedef struct s_ping {
    bool    is_help;
    bool    is_verbose;
    char    *target_host;
} t_ping;

/* Function prototypes */
void print_usage(void);
int  parse_args(int argc, char **argv, t_ping *ctx);

#endif