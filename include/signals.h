#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

extern volatile sig_atomic_t sigusr1_flag;
extern volatile sig_atomic_t sighup_flag;
extern volatile sig_atomic_t alarm_flag;
extern volatile sig_atomic_t exit_mode;

void setup_signals(void);

#endif
