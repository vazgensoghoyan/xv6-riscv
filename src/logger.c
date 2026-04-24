#include "logger.h"
#include <stdlib.h>

static FILE *logf = NULL;

void log_init_foreground(void) {
    logf = stdout;
}

void log_init_file(const char *filename) {
    logf = fopen(filename, "a");
    if (!logf) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
}

void log_msg(const char *msg) {
    if (!logf) return;

    fputs(msg, logf);
    fflush(logf);
}

void log_close(void) {
    if (logf && logf != stdout) {
        fclose(logf);
    }
}
