#include "stats.h"
#include "logger.h"

static long msg_count;
static long bytes_total;
static long alarm_count;

void stats_init(void) {
    msg_count = 0;
    bytes_total = 0;
    alarm_count = 0;
}

void stats_inc_msg(void) {
    msg_count++;
}

void stats_add_bytes(long b) {
    bytes_total += b;
}

void stats_inc_alarm(void) {
    alarm_count++;
}

void stats_print(void) {
    char buf[256];

    snprintf(buf, sizeof(buf),
        "\n[STATS]\nmessages: %ld\nbytes: %ld\nalarms: %ld\n\n",
        msg_count, bytes_total, alarm_count);

    log_msg(buf);
}
