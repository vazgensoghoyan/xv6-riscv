#include "util.h"

#include <unistd.h>

int read_bytes(int fd, void *buf, size_t size, long offset) {
    if (lseek(fd, offset, SEEK_SET) < 0)
        return -1;

    char *p = buf;
    size_t done = 0;

    while (done < size) {
        ssize_t rc = read(fd, p + done, size - done);

        if (rc <= 0)
            return -1;

        done += (size_t)rc;
    }

    return 0;
}
