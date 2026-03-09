#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {

    int pipefd[2];
    
    if (pipe(pipefd) < 0) {
        fprintf(2, "pipe error occured\n");
        exit(1);
    }
    
    int pid = fork();

    if (pid < 0) {
        fprintf(2, "fork error occured\n");
        close(pipefd[0]);
        close(pipefd[1]);
        exit(1);
    }

    if (pid == 0) { // child

        if (close(pipefd[1]) < 0) { // closed writing
            fprintf(2, "close error occured\n");
            exit(1);
        }

        if (close(0) < 0) { // closed stdin
            fprintf(2, "close error occured\n");
            exit(1);
        }
        
        if (dup(pipefd[0]) < 0) {
            fprintf(2, "dup error occured\n");
            close(pipefd[0]);
            exit(1);
        }

        if (close(pipefd[0]) < 0) {
            fprintf(2, "close error occured\n");
            exit(1);
        }

        char *wc_args[] = { "/wc", 0 };
        exec("/wc", wc_args);

        fprintf(2, "exec error occured\n");
        exit(1);
    }

    // parent

    if (close(pipefd[0]) < 0) {  // closed reading
        fprintf(2, "close error occured\n");
        exit(1);
    }

    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        int len = strlen(arg);

        int total_written = 0;

        while (total_written < len) {
            int written = write(pipefd[1], arg + total_written, len - total_written);

            if (written < 0) {
                fprintf(2, "write error occured\n");
                if (close(pipefd[1]) < 0)
                    fprintf(2, "close error occured\n");
                exit(1);
            }

            total_written += written;
        }

        char *newline = "\n";
        total_written = 0;

        while (total_written < 1) {
            int written = write(pipefd[1], newline + total_written, 1 - total_written);

            if (written < 0) {
                fprintf(2, "write error occured\n");
                if (close(pipefd[1]) < 0)
                    fprintf(2, "close error occured\n");
                exit(1);
            }

            total_written += written;
        }
    }

    if (close(pipefd[1]) < 0) {  // closed writing
        fprintf(2, "close error occured\n");
        exit(1);
    }

    int status;
    if (wait(&status) < 0) {
        fprintf(2, "wait error occured\n");
        exit(1);
    }

    exit(0);
}
