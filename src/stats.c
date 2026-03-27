#include "ft_ping.h"

/* * Called every time a valid ping is received.
 * Updates the min, max, and running sums for our final calculations.
 */
void update_stats(t_ping *ctx, double rtt) {
    ctx->stats.packets_received++;
    ctx->stats.rtt_sum += rtt;
    ctx->stats.rtt_sum_sq += (rtt * rtt);

    if (ctx->stats.packets_received == 1) {
        ctx->stats.rtt_min = rtt;
        ctx->stats.rtt_max = rtt;
    } else {
        if (rtt < ctx->stats.rtt_min) ctx->stats.rtt_min = rtt;
        if (rtt > ctx->stats.rtt_max) ctx->stats.rtt_max = rtt;
    }
}

/* * Called when the program exits.
 * Calculates total time, packet loss, and standard deviation (stddev), 
 * then prints the standard ping summary block.
 */
void print_stats(t_ping *ctx) {
    printf("--- %s ping statistics ---\n", ctx->target_host);

    /* Calculate Packet Loss Percentage */
    double loss_pct = 0.0;
    if (ctx->stats.packets_transmitted > 0) {
        loss_pct = ((double)(ctx->stats.packets_transmitted - ctx->stats.packets_received) / 
                   ctx->stats.packets_transmitted) * 100.0;
    }

    /* Print Transmitted / Received / Loss / Total Time */
    printf("%zu packets transmitted, %zu packets received, %.0f%% packet loss\n", 
           ctx->stats.packets_transmitted, 
           ctx->stats.packets_received, 
           loss_pct);

    /* If we actually received packets, calculate and print the min/avg/max/stddev block */
    if (ctx->stats.packets_received > 0) {
        double avg = ctx->stats.rtt_sum / ctx->stats.packets_received;
        
        /* Standard deviation formula: sqrt((sum_of_squares / N) - (average^2)) */
        double variance = (ctx->stats.rtt_sum_sq / ctx->stats.packets_received) - (avg * avg);
        double stddev = sqrt(variance);

        printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", 
               ctx->stats.rtt_min, 
               avg, 
               ctx->stats.rtt_max, 
               stddev);
    }
}
