#include "../ft_ping.h"

void print_usage(void) {
    printf("Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n\n");
    printf("  -v, --verbose              verbose output\n");
    printf("  -?, --help                 give this help list\n");
    printf("\nReport bugs to <bug-inetutils@gnu.org>.\n");
}

int parse_args(int argc, char **argv, t_ping *ctx) {
    bool end_of_options = false;

    for (int i = 1; i < argc; i++) {

        if (!end_of_options && strcmp(argv[i], "--") == 0) {
            end_of_options = true;
            continue; /* Skip the '--' itself and move to the next argument */
        }

        /* Check for long options first */
        else if (!end_of_options && strncmp(argv[i], "--", 2) == 0) {
            if (strcmp(argv[i], "--help") == 0) {
                ctx->is_help = true;
            } else if (strcmp(argv[i], "--verbose") == 0) {
                ctx->is_verbose = true;
            } else {
                fprintf(stderr, "ping: unrecognized option '%s'\n", argv[i]);
                fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
                return (EX_USAGE);
            }
        } 
        /* Check for short concatenated options */
        else if (!end_of_options && argv[i][0] == '-' && argv[i][1] != '\0') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                if (argv[i][j] == 'v') {
                    ctx->is_verbose = true;
                } else if (argv[i][j] == '?') {
                    ctx->is_help = true;
                } else {
                    fprintf(stderr, "ping: invalid option -- '%c'\n", argv[i][j]);
                    fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
                    return (EX_USAGE); 
                }
            }
        } 
        /* It's a positional argument (hostname) */
        else {
            if (ctx->target_host == NULL) {
                ctx->target_host = argv[i];
            }
        }
    }

    /* Post-parsing validation */
    if (ctx->is_help) {
        print_usage();
        /* Return EX_OK (0) so the main program knows to exit cleanly without pinging */
        return (EX_OK); 
    }

    if (ctx->target_host == NULL) {
        fprintf(stderr, "ping: missing host operand\n");
        fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
        return (EX_USAGE); 
    }

    return (EX_OK);
}
