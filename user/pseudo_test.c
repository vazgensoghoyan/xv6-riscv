#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

static int tests_passed = 0;
static int tests_failed = 0;

static void failed(const char* msg) {
    printf("FAILED: %s\n", msg);
    tests_failed++;
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

void test_null(void) {
    printf("\n--- test_null ---\n");

    int fd = open("null", O_WRONLY);
    ASSERT(fd >= 0, "writeonly open failed");

    char *msg = "hello";
    ASSERT(write(fd, msg, 5) == 5, "write failed");

    close(fd);

    fd = open("null", O_RDONLY);
    ASSERT(fd >= 0, "readonly open failed");

    char buf[10];
    int n = read(fd, buf, sizeof(buf));
    ASSERT(n == 0, "read should return zero");

    close(fd);

    PASSED();
}

void test_zero(void) {
    printf("\n--- test_zero ---\n");

    int fd = open("zero", O_RDONLY);
    ASSERT(fd >= 0, "readonly open failed");

    char buf[16];
    int n = read(fd, buf, sizeof(buf));

    int ok = 1;
    for(int i = 0; i < n; i++){
        if(buf[i] != 0)
            ok = 0;
    }

    ASSERT(ok, "read failed, should get all zeros");

    close(fd);

    fd = open("zero", O_WRONLY);
    ASSERT(fd >= 0, "writeonly open failed");

    int w = write(fd, "abc", 3);
    ASSERT(w < 0, "write should fail");

    close(fd);

    PASSED();
}

void test_urandom(void) {
    printf("\n--- test_urandom ---\n");

    int fd = open("urandom", O_WRONLY);
    ASSERT(fd >= 0, "readonly open failed");

    uint64 seed = 0x1122334455667788;
    char bad_seed[3] = {1,2,3};

    ASSERT(write(fd, &seed, sizeof(seed)) == sizeof(seed), "right size seed write shouldnt fail");
    ASSERT(write(fd, &bad_seed, sizeof(bad_seed)) < 0, "wrong size seed write should fail");
    
    close(fd);

    PASSED();
}

void test_nullstat(void) {
    printf("\n--- test_nullstat ---\n");

    int fd = open("nullstat", O_RDWR);
    ASSERT(fd >= 0, "readwrite open failed");

    char buf[4] = {1,2,3,4};

    write(fd, buf, 4);
    write(fd, buf, 2);

    uint64 val;
    read(fd, &val, sizeof(val));

    ASSERT(val == 6, "wrong value got from nullstat");

    close(fd);

    PASSED();
}

int main(void) {
    printf("===== PSEUDO DRIVER TEST START =====\n");

    test_null();
    test_zero();
    test_urandom();
    test_nullstat();

    printf("\n%d TESTS PASSED\n", tests_passed);
    printf("%d TESTS FAILED\n", tests_failed);

    printf("\n===== ALL TESTS DONE =====\n");

    exit(0);
}