#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#include "logger.h"
#include "signals.h"

#define FIFO_PATH "/tmp/log_server.fifo"
#define BUF_SIZE 4096
#define ALARM_SEC 5

static long msg_count = 0;
static long bytes_total = 0;
static long alarm_count = 0;

static int is_daemon = 0;

static void print_stats(void) {
    char buf[256];
    snprintf(buf, sizeof(buf),
             "\n[STATS]\nmessages: %ld\nbytes: %ld\nalarms: %ld\n\n",
             msg_count, bytes_total, alarm_count);
    log_msg(buf);
}

static void ensure_fifo(const char *path) {
    if (mkfifo(path, 0600) == 0) return;

    if (errno != EEXIST) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (stat(path, &st) < 0 || !S_ISFIFO(st.st_mode)) {
        fprintf(stderr, "invalid fifo\n");
        exit(EXIT_FAILURE);
    }
}

static void daemonize(void) {
    if (is_daemon) return;
    is_daemon = 1;

    pid_t pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);

    setsid();

    pid = fork();
    if (pid < 0) exit(1);
    if (pid > 0) exit(0);

    freopen("/tmp/log_server_daemon.log", "a", stdout);
    freopen("/tmp/log_server_daemon.log", "a", stderr);

    log_msg("daemonized via SIGHUP\n");
}

int main(void) {
    log_init_foreground();
    setup_signals();

    log_msg("server started\n");

    ensure_fifo(FIFO_PATH);

    alarm(ALARM_SEC);

    int running = 1;
    int finish_current = 0;

    while (running) {

        if (sigterm_flag) {
            log_msg("SIGTERM -> exit now\n");
            break;
        }

        if (sigusr1_flag) {
            sigusr1_flag = 0;
            print_stats();
        }

        if (sighup_flag) {
            sighup_flag = 0;
            daemonize();
        }

        if (alarm_flag) {
            alarm_flag = 0;
            alarm_count++;
            log_msg("[ALARM] alive\n");
            alarm(ALARM_SEC);
        }

        int fd = open(FIFO_PATH, O_RDONLY);
        if (fd < 0) {
            if (errno == EINTR) continue;
            perror("open");
            exit(1);
        }

        char buf[BUF_SIZE];

        while (1) {

            if (sigterm_flag) {
                close(fd);
                log_msg("SIGTERM -> exit\n");
                goto exit;
            }

            if (sigint_flag) {
                finish_current = 1;
            }

            ssize_t n = read(fd, buf, BUF_SIZE - 1);

            if (n > 0) {
                buf[n] = '\0';
                log_msg(buf);

                if (buf[n - 1] != '\n')
                    log_msg("\n");

                bytes_total += n;
                msg_count++;
                continue;
            }

            if (n == 0) break;

            if (errno == EINTR) {
                continue;
            }

            perror("read");
            close(fd);
            exit(1);
        }

        close(fd);

        if (finish_current) {
            log_msg("SIGINT -> graceful finish\n");
            break;
        }
    }

exit:
    print_stats();
    log_msg("server stopped\n");
    unlink(FIFO_PATH);
    log_close();

    return 0;
}
