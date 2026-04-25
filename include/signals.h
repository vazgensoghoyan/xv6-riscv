#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

void setup_signals(void);

typedef struct {
    volatile sig_atomic_t exit_now;     // SIGTERM
    volatile sig_atomic_t drain_mode;   // SIGINT
    volatile sig_atomic_t alarm_event;
    volatile sig_atomic_t stats_req;
    volatile sig_atomic_t hup_event;
} server_state_t;

extern server_state_t g_state;

#endif
