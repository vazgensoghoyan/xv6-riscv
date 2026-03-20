#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int fail(const char *msg) {
    printf("FAILED: %s\n", msg);
    return 0;
}

int pass(void) {
    printf("PASSED\n");
    return 1;
}

int test_invalid_ops() {
    printf("\n--- test_invalid_ops ---\n");

    int m = mutex();
    if (m < 0) return fail("mutex create");

    char buf[10];

    if (read(m, buf, sizeof(buf)) != -1)
        return fail("read on mutex must return fail");

    if (write(m, buf, sizeof(buf)) != -1)
        return fail("write on mutex must return fail");

    struct stat st;
    if (fstat(m, &st) != -1)
        return fail("fstat on mutex must return fail");

    close(m);

    return pass();
}

int test_cross_unlock() {
    printf("\n--- test_cross_unlock ---\n");

    int m = mutex();
    if (m < 0) return fail("mutex create");

    int pid = fork();
    if (pid < 0) return fail("fork");

    if (pid == 0) {
        if (mutex_unlock(m) == 0)
            return fail("child should not unlock other's mutex");

        exit(0);
    }

    mutex_lock(m);

    wait(0);

    mutex_unlock(m);

    close(m);

    return pass();
}

int test_close_locked() {
    printf("\n--- test_close_locked ---\n");

    int m = mutex();
    if (m < 0) return fail("mutex create");

    mutex_lock(m);

    close(m);

    return pass();
}

int test_fork_shared() {
    printf("\n--- test_fork_shared ---\n");

    int m = mutex();
    if (m < 0) return fail("mutex create");

    mutex_lock(m);

    int pid = fork();
    if (pid < 0) return fail("fork");

    if (pid == 0) {
        pause(1);
        mutex_unlock(m);
        exit(0);
    }

    wait(0);

    mutex_unlock(m);
    close(m);

    return pass();
}

int test_stress() {
    printf("\n--- test_stress ---\n");

    int m = mutex();
    if (m < 0) return fail("mutex create");

    int pid = fork();
    if (pid < 0) return fail("fork");

    for (int i = 0; i < 5; i++) {
        if (pid == 0) {
            mutex_lock(m);
            printf("child  i=%d\n", i);
            mutex_unlock(m);
        } else {
            mutex_lock(m);
            printf("parent i=%d\n", i);
            mutex_unlock(m);
        }
    }

    if (pid == 0)
        exit(0);

    wait(0);
    close(m);

    return pass();
}

int main() {
    printf("\n===== MUTEX TEST START =====\n");

    int cnt = 0;

    cnt += test_invalid_ops();
    cnt += test_cross_unlock();
    cnt += test_close_locked();
    cnt += test_fork_shared();
    cnt += test_stress();

    printf("\n%d TESTS PASSED\n", cnt);

    printf("\n===== ALL TESTS DONE =====\n");

    exit(0);
}
