#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int write_all(int fd, const char *buf, int len) {
    int total = 0;

    while (total < len) {
        int written = write(fd, buf + total, len - total);

        if (written < 0)
            return -1;

        total += written;
    }

    return total;
}

void safe_close(int fd) {
    if (close(fd) < 0) {
        fprintf(2, "close error occured\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {

    int pipefd[2];
    
    if (pipe(pipefd) < 0) {
        fprintf(2, "pipe error occured\n");
        exit(1);
    }
    
    int pid = fork();

    if (pid < 0) {
        fprintf(2, "fork error occured\n");
        safe_close(pipefd[0]);
        safe_close(pipefd[1]);
        exit(1);
    }

    if (pid == 0) { // child

        safe_close(pipefd[1]);  // closed writing

        safe_close(0); // closed stdin

        if (dup(pipefd[0]) < 0) {
            fprintf(2, "dup error occured\n");
            safe_close(pipefd[0]);
            exit(1);
        }

        safe_close(pipefd[0]);

        char *wc_args[] = { "/wc", 0 };
        exec("/wc", wc_args);

        fprintf(2, "exec error occured\n");
        exit(1);
    }

    // parent

    safe_close(pipefd[0]); // closed reading

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        int len = strlen(arg);

        if (write_all(pipefd[1], arg, len) < 0) {
            fprintf(2, "write error occured\n");
            safe_close(pipefd[1]);
            exit(1);
        }

        if (write_all(pipefd[1], "\n", 1) < 0) {
            fprintf(2, "write error occured\n");
            safe_close(pipefd[1]);
            exit(1);
        }
    }

    safe_close(pipefd[1]); // closed writing

    int status;
    if (wait(&status) < 0) {
        fprintf(2, "wait error occured\n");
        exit(1);
    }

    exit(0);
}
