#include "server.h"
#include "logger.h"
#include "stats.h"
#include "signals.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ALARM_SEC 5

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

        alarm(ALARM_SEC);
    }
}

void daemonize_if_needed(void) {

    static int is_daemon = 0;
    if (is_daemon)
        return;

    is_daemon = 1;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0) {
        perror("setsid");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork2");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    if (chdir("/") == -1) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int fd = open("/tmp/log_server_daemon.log",
                  O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (fd < 0) {
        perror("open daemon log");
        exit(EXIT_FAILURE);
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2 stdout");
        exit(EXIT_FAILURE);
    }

    if (dup2(fd, STDERR_FILENO) < 0) {
        perror("dup2 stderr");
        exit(EXIT_FAILURE);
    }

    if (fd > 2) {
        if (close(fd) == -1) {
            perror("close daemon log fd");
            exit(EXIT_FAILURE);
        }
    }

    log_reinit_filepointer(stdout);

    log_msg("daemonized via SIGHUP\n");
}
