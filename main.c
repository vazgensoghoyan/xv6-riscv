#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "logger.h"
#include "signals.h"

#define ALARM_SEC 5

int main(void) {
    log_init_foreground();
    setup_signals();

    log_msg("Log server started\n");

    alarm(ALARM_SEC);

    while (1) {

        // обработка сигналов

        if (sigterm_flag) {
            log_msg("SIGTERM received, exiting immediately\n");
            break;
        }

        if (sigint_flag) {
            log_msg("SIGINT received, exiting after current operation\n");
            break;
        }

        if (sigusr1_flag) {
            sigusr1_flag = 0;
            log_msg("SIGUSR1 received (stats request)\n");
        }

        if (sighup_flag) {
            sighup_flag = 0;
            log_msg("SIGHUP received (daemonize later)\n");
        }

        if (alarm_flag) {
            alarm_flag = 0;
            log_msg("[ALARM] still running...\n");
            alarm(ALARM_SEC);
        }

        // --- блокируемся ---
        int res = pause();

        if (res == -1 && errno == EINTR) {
            // прервались сигналом — это нормально
            continue;
        }
    }

    log_close();
    return 0;
}
