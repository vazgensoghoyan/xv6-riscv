#include "logger.h"
#include "signals.h"
#include "fifo.h"
#include "stats.h"
#include "server.h"

#include <unistd.h>
#include <errno.h>
#include <stdio.h>

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

    while (!g_state.exit_now) {

        handle_async_events();

        if (g_state.exit_now) {
            log_msg("SIGTERM received -> immediate exit\n");
            break;
        }

        int fd = fifo_open_blocking(FIFO_PATH);

        if (fd < 0) {
            if (errno == EINTR)
                continue;

            perror("open fifo");
            break;
        }

        char buf[BUF_SIZE];

        while (!g_state.exit_now) {

            handle_async_events();

            if (g_state.exit_now) {
                log_msg("SIGTERM during FIFO session\n");
                fifo_close(fd);
                goto exit;
            }

            ssize_t n = fifo_read_loop(fd, buf, BUF_SIZE);

            if (n < 0) {
                if (errno == EINTR)
                    continue;

                perror("read fifo");
                fifo_close(fd);
                goto exit;
            }

            if (n == 0)
                break;

            buf[n] = '\0';

            log_msg(buf);

            if (buf[n - 1] != '\n')
                log_msg("\n");

            stats_add_bytes(n);

            if (g_state.drain_mode) {
                log_msg("SIGINT received -> drain mode activated\n");
                break;
            }
        }

        fifo_close(fd);

        stats_inc_msg();

        if (g_state.drain_mode) {
            log_msg("SIGINT drain mode active -> finishing current FIFO, then shutdown\n");
            break;
        }
    }

exit:
    alarm(0);

    stats_print();
    log_msg("server stopped\n");

    unlink(FIFO_PATH);

    log_close();

    return 0;
}
