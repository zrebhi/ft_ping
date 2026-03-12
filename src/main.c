#include "ft_ping.h"

int main(int argc, char **argv) {
    t_ping ping_ctx = {false, false, NULL};
    int status;

    status = parse_args(argc, argv, &ping_ctx);
    
    // If there was an error, or if the user just wanted the help menu, exit immediately.
    if (status != EX_OK || ping_ctx.is_help) {
        return status;
    }

    // Debug output for manual verification
    printf("Target: %s, Verbose: %s\n", ping_ctx.target_host, ping_ctx.is_verbose ? "ON" : "OFF");

    return EX_OK;
}