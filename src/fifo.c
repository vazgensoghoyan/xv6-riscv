#include "fifo.h"
#include "signals.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int fifo_open_blocking(const char *path) {

    int fd;

    while (1) {

        if (g_state.exit_now)
            return -1;

        fd = open(path, O_RDONLY);

        if (fd < 0) {

            if (errno == EINTR)
                continue;

            perror("open fifo");
            return -1;
        }

        return fd;
    }
}

int fifo_read_loop(int fd, char *buf, int size) {
    ssize_t r;

    while (1) {
        r = read(fd, buf, size - 1);

        if (r == -1 && errno == EINTR)
            continue;

        return r;
    }
}

void fifo_close(int fd) {
    if (fd >= 0)
        close(fd);
}
