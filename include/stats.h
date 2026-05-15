#ifndef STATS_H
#define STATS_H

void stats_init(void);
void stats_inc_msg(void);
void stats_add_bytes(long bytes);
void stats_inc_alarm(void);

void stats_print(void);

#endif
