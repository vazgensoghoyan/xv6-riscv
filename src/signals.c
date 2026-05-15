#include "signals.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction SIGUSR1");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction SIGHUP");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction SIGALRM");
        exit(EXIT_FAILURE);
    }

    struct sigaction ign;
    memset(&ign, 0, sizeof(ign));
    ign.sa_handler = SIG_IGN;

    if (sigaction(SIGQUIT, &ign, NULL) == -1) {
        perror("sigaction SIGQUIT");
        exit(EXIT_FAILURE);
    }
}
