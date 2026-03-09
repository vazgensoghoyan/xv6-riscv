#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define BUF_SIZE 8192

ssize_t write_all(int fd, const char *buf, size_t len) {
    ssize_t total = 0;
    while (total < (ssize_t)len) {
        ssize_t written = write(fd, buf + total, len - total);
        if (written < 0)
            return -1;
        total += written;
    }
    return total;
}

void safe_close(int fd) {
    if (close(fd) < 0) {
        perror("close error");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]) {

    int pipefd[2];

    if (pipe(pipefd) < 0) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork error");
        safe_close(pipefd[0]);
        safe_close(pipefd[1]);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // child

        safe_close(pipefd[1]);

        char buffer[BUF_SIZE] = { 0 };
        ssize_t bytes_read;

        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            if (write_all(STDOUT_FILENO, buffer, bytes_read) < 0) {
                perror("write error");
                safe_close(pipefd[0]);
                exit(EXIT_FAILURE);
            }
        }

        if (bytes_read < 0) {
            perror("read error");
            safe_close(pipefd[0]);
            exit(EXIT_FAILURE);
        }

        safe_close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }

    // parent

    safe_close(pipefd[0]);

    for (int i = 1; i < argc; i++) {
        size_t len = strlen(argv[i]);
        if (write_all(pipefd[1], argv[i], len) < 0) {
            perror("write error");
            safe_close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        char newline = '\n';
        if (write_all(pipefd[1], &newline, 1) < 0) {
            perror("write error");
            safe_close(pipefd[1]);
            exit(EXIT_FAILURE);
        }
    }

    safe_close(pipefd[1]);

    if (wait(NULL) < 0) {
        perror("wait error");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
