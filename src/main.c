#include "ft_ping.h"

int main(int argc, char **argv) {
    t_ping ping_ctx = {0}; // Zero-initialize context
    int status;

    status = parse_args(argc, argv, &ping_ctx);
    if (status != EX_OK || ping_ctx.is_help) {
        return status;
    }

    status = resolve_dns(&ping_ctx);
    if (status != 0) {
        return status;
    }

    status = create_socket(&ping_ctx);
    if (status != 0) {
        return status;
    }

    printf("PING %s (%s): %d data bytes\n", 
            ping_ctx.target_host, 
            ping_ctx.dest_ip, 
            PING_DATA_SIZE);

    return EX_OK;
}
