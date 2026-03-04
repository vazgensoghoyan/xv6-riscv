#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define BUF_SIZE 8192

int main(int argc, char* argv[]) {

    int pipefd[2];
    
    if (pipe(pipefd) < 0) {
        fprintf(stderr, "pipe error occured\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork error occured\n");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // child

        close(pipefd[1]);

        char buffer[BUF_SIZE] = { 0 };
        ssize_t bytes_read;

        do {
            bytes_read = read(pipefd[0], buffer, sizeof(buffer));
    
            if (bytes_read < 0) {
                fprintf(stderr, "read error occured\n");
                close(pipefd[0]);
                exit(EXIT_FAILURE);
            }

            // if bytes_read == 0 then while is ignored :)
            // rmk: i was not sure if we can use 'printf'
            // thats why 'write'

            ssize_t total_written = 0;
            while (total_written < bytes_read) {
                ssize_t bytes_written = write(
                    STDOUT_FILENO,
                    buffer + total_written,
                    bytes_read - total_written
                );

                if (bytes_written < 0) {
                    fprintf(stderr, "write error occured\n");
                    close(pipefd[0]);
                    exit(EXIT_FAILURE);
                }

                total_written += bytes_written;
            }
        } while (bytes_read);

        close(pipefd[0]);

        exit(EXIT_SUCCESS);
    }

    // parent

    close(pipefd[0]);

    for (int i = 1; i < argc; i++) {
        size_t len = strlen(argv[i]);
        size_t total_written = 0;

        while (total_written < len) {
            ssize_t bytes_written = write(
                pipefd[1],
                argv[i] + total_written,
                len - total_written
            );

            if (bytes_written < 0) {
                fprintf(stderr, "write error occured\n");
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }

            total_written += bytes_written;
        }

        char newline = '\n';
        total_written = 0;
        
        do {
            total_written = write(pipefd[1], &newline, 1); // this can also return 0 :)

            if (total_written < 0) {
                fprintf(stderr, "write error occured\n");
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }

        } while (total_written == 0); // we need only 1 bite to be written
    }

    close(pipefd[1]);

    if (wait(NULL) < 0) {
        perror("wait");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
