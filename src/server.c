#include "server.h"
#include "logger.h"
#include "stats.h"
#include "signals.h"

#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

void ensure_fifo(const char *path) {
    struct stat st;

    if (mkfifo(path, 0600) == -1) {
        if (errno == EEXIST) {
            if (stat(path, &st) == -1) {
                perror("stat");
                exit(1);
            }

            if (!S_ISFIFO(st.st_mode)) {
                fprintf(stderr, "Error: %s exists but is not FIFO\n", path);
                exit(1);
            }

            return;
        }

        perror("mkfifo");
        exit(1);
    }
}

void handle_async_events(void) {

    if (g_state.stats_req) {
        g_state.stats_req = 0;
        stats_print();
    }

    if (g_state.hup_event) {
        g_state.hup_event = 0;
        daemonize_if_needed();
    }

    if (g_state.alarm_event) {
        g_state.alarm_event = 0;

        stats_inc_alarm();
        log_msg("[ALARM] server running, waiting for data\n");

        alarm(5);
    }
}

void daemonize_if_needed(void) {
    static int is_daemon = 0;

    if (is_daemon)
        return;

    is_daemon = 1;

    if (fork() > 0) exit(0);
    setsid();

    if (fork() > 0) exit(0);

    FILE *f = fopen("/tmp/log_server_daemon.log", "a");
    if (!f) exit(1);

    log_init_file("/tmp/log_server_daemon.log");
    log_reinit_filepointer(f);

    dup2(fileno(f), STDOUT_FILENO);
    dup2(fileno(f), STDERR_FILENO);

    log_msg("daemonized via SIGHUP\n");
}
