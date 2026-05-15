#include "logger.h"
#include "signals.h"
#include "stats.h"
#include "server.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define FIFO_PATH "/tmp/log_server.fifo"
#define BUF_SIZE 4096
#define ALARM_SEC 5

static void cleanup_fd(int fd) {
    if (fd >= 0) {
        if (close(fd) == -1)
            perror("close");
    }
}

// EVENT PHASE

static void process_events(void) {
    handle_async_events();

    if (g_state.exit_now)
        log_msg("SIGTERM received -> exit\n");

    if (g_state.drain_mode)
        log_msg("SIGINT -> drain mode active\n");
}

// FIFO OPEN PHASE

static int open_fifo(const char *path) {
    int fd;

    while (1) {
        fd = open(path, O_RDONLY);
        if (fd >= 0)
            return fd;

        if (errno == EINTR) {
            process_events();
            if (g_state.exit_now || g_state.drain_mode)
                return -1;
            continue;
        }

        perror("open fifo");
        return -1;
    }
}

// READ PHASE

static int process_fifo(int fd) {
    char buf[BUF_SIZE];

    while (1) {
        ssize_t n = read(fd, buf, BUF_SIZE - 1);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("read fifo");
            return -1;
        }

        if (n == 0) break;

        buf[n] = '\0';

        log_msg(buf);

        if (buf[n - 1] != '\n')
            log_msg("\n");

        stats_add_bytes(n);
        process_events();

        if (g_state.exit_now)
            return -1;

        if (g_state.drain_mode)
            break;
    }

    return 0;
}

// MAIN

int main(void) {

    log_init_foreground();
    setup_signals();

    ensure_fifo(FIFO_PATH);
    stats_init();

    alarm(ALARM_SEC);

    log_msg("server started\n");

    while (!g_state.exit_now) {

        process_events();

        if (g_state.exit_now)
            break;

        int fd = open_fifo(FIFO_PATH);

        if (fd < 0) {
            if (g_state.exit_now) break;

            if (g_state.drain_mode) break;

            continue;
        }

        int rc = process_fifo(fd);

        cleanup_fd(fd);

        if (rc < 0) break;

        stats_inc_msg();

        if (g_state.drain_mode) {
            log_msg("drain completed -> shutdown\n");
            break;
        }
    }

    alarm(0);

    stats_print();
    log_msg("server stopped\n");

    if (unlink(FIFO_PATH) == -1)
        perror("unlink");

    log_close();

    return EXIT_SUCCESS;
}
