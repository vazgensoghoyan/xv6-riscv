#include "server.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

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
