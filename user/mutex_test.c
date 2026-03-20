#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

static int tests_passed = 0;

static void failed(const char* msg) {
    printf("FAILED: %s\n", msg);
}

static void passed(void) {
    printf("PASSED\n");
    tests_passed++;
}

#define ASSERT(cond, msg) \
    if (!(cond)) { \
        failed(msg); \
        return; \
    }

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

    mutex_lock(m);

    int pid = fork();
    ASSERT(pid >= 0, "fork failed");

    if (pid == 0) {
        int r = mutex_unlock(m);
        if (r == 0) {
            failed("child unlocked other's mutex\n");
            exit(1);
        }
        exit(0);
    }

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

    int pid = fork();
    ASSERT(pid >= 0, "fork failed");

    for (int i = 0; i < 5; i++) {
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

    PASSED();
}

void test_stress(void) {
    printf("\n--- test_stress ---\n");

    int m = mutex();
    ASSERT(m >= 0, "mutex create");

    int pid1 = fork();
    ASSERT(pid1 >= 0, "fork");
    if (pid1 == 0) {
        for (int i = 0; i < 10; i++) {
            mutex_lock(m);
            printf("proc %d i=%d\n", getpid(), i);
            mutex_unlock(m);
        }
        exit(0);
    }

    int pid2 = fork();
    ASSERT(pid2 >= 0, "fork");
    if (pid2 == 0) {
        for (int i = 0; i < 10; i++) {
            mutex_lock(m);
            printf("proc %d i=%d\n", getpid(), i);
            mutex_unlock(m);
        }
        exit(0);
    }

    for (int i = 0; i < 10; i++) {
        mutex_lock(m);
        printf("proc %d i=%d\n", getpid(), i);
        mutex_unlock(m);
    }

    int s1, s2;
    ASSERT(wait(&s1) > 0, "wait failed");
    ASSERT(wait(&s2) > 0, "wait failed");

    close(m);

    PASSED();
}

void test_exit_holding_lock(void) {
    printf("\n--- test_exit_holding_lock ---\n");
    int m = mutex();
    ASSERT(m >= 0, "mutex create");
    int pid = fork();
    ASSERT(pid >= 0, "fork");
    if (pid == 0) {
        mutex_lock(m);
        exit(0);
    }
    wait(0);
    mutex_lock(m);
    mutex_unlock(m);
    close(m);
    PASSED();
}

void test_close_by_other(void) {
    printf("\n--- test_close_by_other ---\n");
    int m = mutex();
    ASSERT(m >= 0, "mutex create");
    int pid = fork();
    ASSERT(pid >= 0, "fork");
    if (pid == 0) {
        close(m);
        exit(0);
    }
    wait(0);
    mutex_lock(m);
    mutex_unlock(m);
    close(m);
    PASSED();
}

int main(void) {
    printf("===== MUTEX TEST START =====\n");

    test_invalid_ops();
    test_cross_unlock();
    test_close_locked();
    test_fork_shared();
    test_stress();
    test_exit_holding_lock();
    test_close_by_other();

    printf("\n%d TESTS PASSED\n", tests_passed);

    printf("\n===== ALL TESTS DONE =====\n");

    exit(0);
}
