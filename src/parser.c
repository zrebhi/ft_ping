#include "ft_ping.h"

void print_usage(void) {
    printf("Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n\n");
    printf("  -v, --verbose              verbose output\n");
    printf("  -?, --help                 give this help list\n");
    printf("\nReport bugs to <bug-inetutils@gnu.org>.\n");
}

int parse_args(int argc, char **argv, t_ping *ctx) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--help") == 0) {
                print_usage();
                ctx->is_help = true;
                return EX_OK;
            } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
                ctx->is_verbose = true;
            } else {
                fprintf(stderr, "ping: invalid option -- '%c'\n", argv[i][1]);
                fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
                return EX_USAGE; 
            }
        } else {
            if (ctx->target_host == NULL) {
                ctx->target_host = argv[i];
            }
        }
    }

    if (ctx->target_host == NULL) {
        fprintf(stderr, "ping: missing host operand\n");
        fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
        return EX_USAGE; 
    }

    return EX_OK;
}