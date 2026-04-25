#include "signals.h"
#include <stddef.h>

server_state_t g_state = {0};

static void handler(int sig) {
    switch (sig) {
        case SIGINT:
            g_state.drain_mode = 1;
            break;
        case SIGTERM:
            g_state.exit_now = 1;
            break;
        case SIGUSR1:
            g_state.stats_req = 1;
            break;
        case SIGHUP:
            g_state.hup_event = 1;
            break;
        case SIGALRM:
            g_state.alarm_event = 1;
            break;
    }
}

void setup_signals(void) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // важно: NO SA_RESTART

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    signal(SIGQUIT, SIG_IGN);
}
