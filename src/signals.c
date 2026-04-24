#include "signals.h"
#include <string.h>

volatile sig_atomic_t sigint_flag = 0;
volatile sig_atomic_t sigterm_flag = 0;
volatile sig_atomic_t sigusr1_flag = 0;
volatile sig_atomic_t sighup_flag = 0;
volatile sig_atomic_t alarm_flag = 0;

static void handler(int signo) {
    if (signo == SIGINT) sigint_flag = 1;
    else if (signo == SIGTERM) sigterm_flag = 1;
    else if (signo == SIGUSR1) sigusr1_flag = 1;
    else if (signo == SIGHUP) sighup_flag = 1;
    else if (signo == SIGALRM) alarm_flag = 1;
}

void setup_signals(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handler;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    signal(SIGQUIT, SIG_IGN);
}
