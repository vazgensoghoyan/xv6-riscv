

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#include "logger.h"
#include "signals.h"
#include "fifo.h"
#include "stats.h"
#include "daemon.h"
#include "server.h"

#define FIFO_PATH "/tmp/log_server.fifo"
#define BUF_SIZE 4096
#define ALARM_SEC 5

int main(void) {
    log_init_foreground();
    setup_signals();

    ensure_fifo(FIFO_PATH);
    stats_init();

    alarm(ALARM_SEC);

    log_msg("server started\n");

    while (1) {

        if (exit_mode == 2) {
            log_msg("SIGTERM -> exit\n");
            break;
        }

        if (sigusr1_flag) {
            sigusr1_flag = 0;
            stats_print();
        }

        if (sighup_flag) {
            sighup_flag = 0;
            daemonize_if_needed();
        }

        if (alarm_flag) {
            alarm_flag = 0;
            stats_inc_alarm();
            log_msg("[ALARM] alive\n");
            alarm(ALARM_SEC);
        }

        int fd = fifo_open_blocking(FIFO_PATH);
        if (fd < 0)
            goto exit;

        char buf[BUF_SIZE];

        while (1) {

            ssize_t n = fifo_read_loop(fd, buf, BUF_SIZE);

            if (n == 0)
                break;

            if (n > 0) {
                buf[n] = '\0';

                log_msg(buf);

                if (buf[n - 1] != '\n')
                    log_msg("\n");

                stats_add_bytes(n);
                stats_inc_msg();
                continue;
            }

            if (n < 0 && errno == EINTR)
                continue;

            perror("read");
            close(fd);
            exit(1);
        }

        close(fd);

        if (exit_mode == 1) {
            log_msg("SIGINT drain done\n");
            break;
        }
    }

exit:
    stats_print();
    log_msg("server stopped\n");

    unlink(FIFO_PATH);
    log_close();

    return 0;
}
