#include "signals.h"
#include <string.h>

volatile sig_atomic_t sigusr1_flag = 0;
volatile sig_atomic_t sighup_flag = 0;
volatile sig_atomic_t alarm_flag = 0;
volatile sig_atomic_t exit_mode = 0;

static void handler(int signo) {
    switch (signo) {
        case SIGINT:
            exit_mode = 1;   // drain mode
            break;
        case SIGTERM:
            exit_mode = 2;   // immediate exit
            break;
        case SIGUSR1:
            sigusr1_flag = 1;
            break;
        case SIGHUP:
            sighup_flag = 1;
            break;
        case SIGALRM:
            alarm_flag = 1;
            break;
    }
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
