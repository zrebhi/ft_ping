#include "../ft_ping.h"

void print_usage(void) {
    printf("Usage: ping [OPTION...] HOST ...\n");
    printf("Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    printf(" Options valid for all request types:\n\n");
    printf("  -c, --count=NUMBER         stop after sending NUMBER packets\n");
    printf("  -q, --quiet                quiet output\n");
    printf("  -r, --ignore-routing       send directly to a host on an attached network\n");
    printf("      --ttl=N                specify N as time-to-live\n");
    printf("  -w, --timeout=N            stop after N seconds\n");
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

/* Mimics inetutils 2.0 -c flag parsing: uses strtoul, catches garbage, but allows overflow/underflow */
static int extract_count(const char *val_str, size_t *target) {
    char *endptr;
    unsigned long val;

    errno = 0;
    val = strtoul(val_str, &endptr, 0);
    
    /* If no conversion was performed or there are leftover characters, it's garbage */
    if (val_str == endptr || *endptr != '\0') {
        fprintf(stderr, "ping: invalid value (`%s' near `%s')\n", val_str, endptr);
        return EX_USAGE;
    }
    
    *target = (size_t)val;
    return EX_OK;
}


/* Safely consumes the next argv element if available, updating the loop index */
static char *consume_next_arg(char **argv, int *i, int argc, const char *flag_display, bool is_short_opt) {
    if (*i + 1 < argc) {
        (*i)++;
        return argv[*i];
    }
    
    if (is_short_opt) {
        fprintf(stderr, "ping: option requires an argument -- '%s'\n", flag_display);
    } else {
        fprintf(stderr, "ping: option '%s' requires an argument\n", flag_display);
    }
    
    fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
    return NULL;
}

/* Extracts the value from an option, advancing the argv index if necessary */
static char *get_opt_val(char *current_arg, char **argv, int *i, int argc, const char *flag_name) {
    char *eq_ptr = strchr(current_arg, '=');
    
    /* Case A: Value is attached via '=' (e.g., --ttl=64) */
    if (eq_ptr) {
        return eq_ptr + 1; 
    }
    
    /* Case B & C: Space-separated value or Missing value */
    return consume_next_arg(argv, i, argc, flag_name, false);
}

/* Checks if the argument strictly matches the flag, or matches the flag + '=' */
static bool is_long_opt(const char *arg, const char *flag) {
    size_t len = strlen(flag);
    return (strncmp(arg, flag, len) == 0 && (arg[len] == '\0' || arg[len] == '='));
}

static int parse_long_opt(char **argv, int *i, int argc, t_ping *ctx) {
    char *arg = argv[*i];

    if (is_long_opt(arg, "--help")) {
        ctx->is_help = true;
    } else if (is_long_opt(arg, "--verbose")) {
        ctx->is_verbose = true;
    } else if (is_long_opt(arg, "--quiet")) {
        ctx->is_quiet = true;
    } else if (is_long_opt(arg, "--ignore-routing")) {
        ctx->is_ignore_routing = true;
    } else if (is_long_opt(arg, "--ttl")) {
        char *val = get_opt_val(arg, argv, i, argc, "--ttl");
        if (!val || extract_numeric(val, 1, 255, &ctx->ttl) != EX_OK) return EX_USAGE;
    } else if (is_long_opt(arg, "--timeout")) {
        char *val = get_opt_val(arg, argv, i, argc, "--timeout");
        if (!val || extract_numeric(val, 1, INT_MAX, &ctx->timeout) != EX_OK) return EX_USAGE;
    } else if (is_long_opt(arg, "--count")) {
        char *val = get_opt_val(arg, argv, i, argc, "--count");
        if (!val || extract_count(val, &ctx->count) != EX_OK) return EX_USAGE;
    } else {
        fprintf(stderr, "ping: unrecognized option '%s'\n", arg);
        return EX_USAGE;
    }
    return EX_OK;
}

static int parse_short_opt(char **argv, int *i, int argc, t_ping *ctx) {
    char *arg = argv[*i];
    int j = 1; /* Start at 1 to skip the '-' */

    while (arg[j] != '\0') {
        char flag = arg[j];

        /* Handle Boolean Flags */
        if (flag == 'v') {
            ctx->is_verbose = true;
        } else if (flag == 'q') {
            ctx->is_quiet = true;
        } else if (flag == 'r') {
            ctx->is_ignore_routing = true;
        } else if (flag == '?') {
            ctx->is_help = true;
        } 
        /* Handle Flags Requiring Arguments */
        else if (flag == 'c' || flag == 'w') {
            char *val_str = NULL;
            
            /* Case A: Concatenated value (e.g., -c3 or -vqc3) */
            if (arg[j + 1] != '\0') {
                val_str = &arg[j + 1];
            } 
            /* Case B & C: Space-separated value or Missing value */
            else {
                char flag_str[2] = {flag, '\0'};
                val_str = consume_next_arg(argv, i, argc, flag_str, true);
                if (!val_str) return EX_USAGE;
            }

            /* Route the extracted string to the correct numeric parser */
            if (flag == 'c') {
                if (extract_count(val_str, &ctx->count) != EX_OK) return EX_USAGE;
            } else if (flag == 'w') {
                if (extract_numeric(val_str, 1, INT_MAX, &ctx->timeout) != EX_OK) return EX_USAGE;
            }

            /* The rest of this arg string was just consumed as a value, so stop looping */
            break; 
        } 
        else {
            fprintf(stderr, "ping: invalid option -- '%c'\n", flag);
            fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
            return EX_USAGE;
        }
        
        j++;
    }
    
    return EX_OK;
}

int parse_args(int argc, char **argv, t_ping *ctx) {
    bool end_of_options = false;

    for (int i = 1; i < argc; i++) {
        /* Handle the explicit end-of-options marker '--' */
        if (!end_of_options && strcmp(argv[i], "--") == 0) {
            end_of_options = true;
        } 
        /* Handle Long Options (e.g., --ttl=64) */
        else if (!end_of_options && strncmp(argv[i], "--", 2) == 0) {
            if (parse_long_opt(argv, &i, argc, ctx) != EX_OK) {
                return EX_USAGE;
            }
        } 
        /* Handle Short Options (e.g., -vqc3) */
        else if (!end_of_options && argv[i][0] == '-' && argv[i][1] != '\0') {
            if (parse_short_opt(argv, &i, argc, ctx) != EX_OK) {
                return EX_USAGE;
            }
        } 
        /* Handle Positional Arguments (the target hostname) */
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
        return EX_OK; 
    }

    if (ctx->target_host == NULL) {
        fprintf(stderr, "ping: missing host operand\n");
        fprintf(stderr, "Try 'ping --help' or 'ping --usage' for more information.\n");
        return EX_USAGE; 
    }

    return EX_OK;
}
