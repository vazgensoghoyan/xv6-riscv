#include "fifo.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

extern volatile sig_atomic_t exit_mode;

int fifo_open_blocking(const char *path) {
    int fd;

    while (1) {
        if (exit_mode == 2)
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

int fifo_read_loop(int fd, char *buf, int buf_size) {
    return read(fd, buf, buf_size - 1);
}

void fifo_close(int fd) {
    if (fd >= 0)
        close(fd);
}
