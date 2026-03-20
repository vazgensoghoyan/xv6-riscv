#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

static int tests_passed = 0;

static void failed(const char* msg) {
    printf("FAILED: %s\n", msg);
}

static void passed() {
    printf("PASSED\n");
    tests_passed++;
}

#define ASSERT(cond, msg) \
    if (!(cond)) { failed(msg); return; } // returning and printing FAILED: msg

#define PASSED() passed(); // wanted this way for beauty

void test_invalid_ops(void) {
    printf("\n--- test_invalid_ops ---\n");

    int m = mutex();
    ASSERT(m >= 0, "mutex create failed");

    char buf[10];

    ASSERT(read(m, buf, sizeof(buf)) == -1, "read must fail");
    ASSERT(write(m, buf, sizeof(buf)) == -1, "write must fail");

    struct stat st;
    ASSERT(fstat(m, &st) == -1, "fstat must fail");

    close(m);

    PASSED();
}

void test_cross_unlock(void) {
    printf("\n--- test_cross_unlock ---\n");

    int m = mutex();
    ASSERT(m >= 0, "mutex create failed");

    int pid = fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        ASSERT(mutex_unlock(m) != 0, "child must NOT unlock mutex");
        exit(0);
    }

    mutex_lock(m);

    int status;
    ASSERT(wait(&status) == pid, "wait failed");

    mutex_unlock(m);
    close(m);

    PASSED();
}

void test_close_locked(void) {
    printf("\n--- test_close_locked ---\n");

    int m = mutex();
    ASSERT(m >= 0, "mutex create failed");

    mutex_lock(m);

    close(m);

    PASSED();
}

void test_fork_shared(void) {
    printf("\n--- test_fork_shared ---\n");

    int m = mutex();
    ASSERT(m >= 0, "mutex create failed");

    mutex_lock(m);

    int pid = fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        pause(2);
        mutex_unlock(m);
        exit(0);
    }

    int status;
    ASSERT(wait(&status) == pid, "wait failed");

    mutex_unlock(m);
    close(m);

    PASSED();
}

void test_stress(void) {
    printf("\n--- test_stress ---\n");

    int m = mutex();
    ASSERT(m >= 0, "mutex create failed");

    int pid = fork();
    ASSERT(pid >= 0, "fork failed");

    for (int i = 0; i < 10; i++) {
        mutex_lock(m);

        if (pid == 0)
            printf("child  i=%d\n", i);
        else
            printf("parent i=%d\n", i);

        mutex_unlock(m);
    }

    if (pid == 0)
        exit(0);

    int status;
    ASSERT(wait(&status) == pid, "wait failed");

    close(m);

    passed();
}

int main(void) {
    printf("===== MUTEX TEST START =====\n");

    test_invalid_ops();
    test_cross_unlock();
    test_close_locked();
    test_fork_shared();
    test_stress();

    printf("\n%d TESTS PASSED\n", tests_passed);

    printf("\n===== ALL TESTS DONE =====\n");

    exit(0);
}
