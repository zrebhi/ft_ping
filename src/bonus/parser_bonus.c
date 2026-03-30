#include "../ft_ping.h"

void print_usage(void) {
    printf("Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n\n");
    printf("  -r, --ignore-routing       send directly to a host on an attached network\n");
    printf("  -v, --verbose              verbose output\n");
    printf("  -?, --help                 give this help list\n");
    printf("\nReport bugs to <bug-inetutils@gnu.org>.\n");
}

/* Generic helper to extract and validate numeric flag arguments.
 * Handles standard ranges and exact inetutils-2.0 error messaging.
 */
static int extract_numeric(const char *val_str, int min, int max, int *target) {
    char *endptr;
    unsigned long val;
    int saved_errno = errno;

    /* Reset errno to distinguish success/failure after the call
     * That's a standrad practice before using strtol */
    errno = 0;
    
    /* Base 0 allows for standard base-10, as well as hex (0x) and octal (0) */
    val = strtol(val_str, &endptr, 0);

    /* Check for no digits found or trailing garbage
     * If endptr points to the original string, no conversion was performed.
     * If endptr is not '\0', there were leftover unparsed characters.
     */
    if (val_str == endptr || *endptr != '\0') {
        /* Matches inetutils: ping: invalid value (`abc' near `abc') */
        fprintf(stderr, "ping: invalid value (`%s' near `%s')\n", val_str, endptr);
        errno = saved_errno; /* Restore the original errno before returning */
        return EX_USAGE;
    }

    /* Cast to unsigned long to replicate inetutils behavior */
    if (val < (unsigned long)min) {
        fprintf(stderr, "ping: option value too small: %s\n", val_str);
        errno = saved_errno;
        return EX_USAGE;
    }

    if (val > (unsigned long)max) {
        fprintf(stderr, "ping: option value too big: %s\n", val_str);
        errno = saved_errno;
        return EX_USAGE;
    }

    *target = (int)val;
    errno = saved_errno;
    return EX_OK;
}

int parse_args(int argc, char **argv, t_ping *ctx) {
    bool end_of_options = false;

    for (int i = 1; i < argc; i++) {

        if (!end_of_options && strcmp(argv[i], "--") == 0) {
            end_of_options = true;
            continue; /* Skip the '--' itself and move to the next argument */
        }

        else if (!end_of_options && strncmp(argv[i], "--", 2) == 0) {
            if (strcmp(argv[i], "--help") == 0) {
                ctx->is_help = true;
            } else if (strcmp(argv[i], "--verbose") == 0) {
                ctx->is_verbose = true;
            } else if (strncmp(argv[i], "--ttl=", 6) == 0) {
                if (extract_numeric(argv[i] + 6, 1, 255, &ctx->ttl) != EX_OK)
                return (EX_USAGE);
            } else if (strcmp(argv[i], "--ttl") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "ping: option '--ttl' requires an argument\n");
                    fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
                    return (EX_USAGE);
                }
                if (extract_numeric(argv[++i], 1, 255, &ctx->ttl) != EX_OK)
                return EX_USAGE;
            } else if (strcmp(argv[i], "--ignore-routing") == 0) {
                ctx->is_ignore_routing = true;
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
                } else if (argv[i][j] == 'r') {
                    ctx->is_ignore_routing = true;
                }
                else {
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
