#include "util.h"

#include <unistd.h>
#include <sys/types.h>

int read_bytes(int fd, void *buf, size_t size, off_t offset) {
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
