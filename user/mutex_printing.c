#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

// Печать аргументов с возможной блокировкой мьютекса
void print_args(int mutex_fd, int argc, char *argv[]) {
    int pid = getpid();
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        for (int j = 0; arg[j] != '\0'; j++) {
            if (mutex_fd >= 0)
                mutex_lock(mutex_fd); // захват перед строкой

            printf("%d: arg %d, char '%c'\n", pid, i, arg[j]);

            if (mutex_fd >= 0)
                mutex_unlock(mutex_fd); // освобождение после строки
        }
    }
}

void fail() {
    fprintf(2, "something went wrong\n");
    exit(1);
}

void test_without_mutex(int argc, char* argv[]) {
    printf("\n=== Without mutex ===\n");
    int pid1, pid2;

    if ((pid1 = fork()) < 0) fail();
    if (pid1 == 0) {
        print_args(-1, argc, argv);
        exit(0);
    }

    if ((pid2 = fork()) < 0) fail();
    if (pid2 == 0) {
        print_args(-1, argc, argv);
        exit(0);
    }

    wait(0);
    wait(0);

    printf("\n");
}

void test_with_mutex(int argc, char* argv[]) {
    printf("\n=== With mutex ===\n");

    int mfd = mutex();
    int pid1, pid2;

    if (mfd < 0) fail();

    if ((pid1 = fork()) < 0) fail();
    if (pid1 == 0) {
        print_args(mfd, argc, argv);
        exit(0);
    }

    if ((pid2 = fork()) < 0) fail();
    if (pid2 == 0) {
        print_args(mfd, argc, argv);
        exit(0);
    }

    wait(0);
    wait(0);

    close(mfd);

    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s args...\n", argv[0]);
        exit(1);
    }

    test_without_mutex(argc, argv);    
    test_with_mutex(argc, argv);    

    exit(0);
}
