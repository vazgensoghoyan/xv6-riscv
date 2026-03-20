#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

// Печать аргументов с возможной блокировкой мьютекса
void print_args(int mutex_fd, int argc, char *argv[]) {
    int pid = getpid();

    for (int i = 1; i < argc; i++) {
        for (int j = 0; argv[i][j] != '\0'; j++) {

            if (mutex_fd >= 0)
                mutex_lock(mutex_fd); // захват перед строкой

            printf("%d: arg %d, char '%c'\n", pid, i, argv[i][j]);

            if (mutex_fd >= 0)
                mutex_unlock(mutex_fd); // освобождение после строки
        }
    }
}

void fail() {
    fprintf(2, "something went wrong\n");
    exit(1);
}

void test(int argc, char* argv[], int with_mutex) {

    if (with_mutex)
        printf("\n=== With mutex ===\n");
    else
        printf("\n=== Without mutex ===\n");

    int pid1, pid2;
    int mutex_fd = -1;

    if (with_mutex) {
        mutex_fd = mutex();
        if (mutex_fd < 0) fail();
    }

    if ((pid1 = fork()) < 0) fail();
    if (pid1 == 0) {
        print_args(mutex_fd, argc, argv);
        if (with_mutex) close(mutex_fd);
        exit(0);
    }

    if ((pid2 = fork()) < 0) fail();
    if (pid2 == 0) {
        print_args(mutex_fd, argc, argv);
        if (with_mutex) close(mutex_fd);
        exit(0);
    }

    wait(0);
    wait(0);

    if (with_mutex) close(mutex_fd); // закрываем мьютекс в последний раз

    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s args...\n", argv[0]);
        exit(1);
    }

    test(argc, argv, 0); // without mutex    
    test(argc, argv, 1); // with mutex

    exit(0);
}
