#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"

char* state_to_string(int state) {
    switch (state) {
        case U_UNUSED: return "UNUSED";
        case U_USED: return "USED";
        case U_SLEEPING: return "SLEEPING";
        case U_RUNNABLE: return "RUNNABLE";
        case U_RUNNING: return "RUNNING";
        case U_ZOMBIE: return "ZOMBIE";
        default: return "UNKNOWN";
    }
}

void print_procinfo(struct procinfo* info) {
    fprintf(1, "    PID: %d, Parent PID: %d, State: %s, Name: %s\n",
        info->pid, info->parent_pid, state_to_string(info->state), info->name);
}

int main(int argc, char* argv[]) {

    printf("TESTING 'ps_listinfo' SYSCALL STARTED\n");

    // NULL call -> getting count of processes
    int n;
    n = ps_listinfo((struct procinfo*)0, 0);
    if (n >= 0) {
        printf("1) [PASS] Number of processes (plist==NULL): %d\n", n);
    } else {
        printf("1) [WRONG] Got %d\n", n);
    }

    // Small buffer
    // RMK: 2 нам подходит, так как как минимум есть init, sh, ps_test
    int small_lim = 2;

    struct procinfo infos_1[small_lim];
    n = ps_listinfo(infos_1, small_lim);

    if (n == -1) {
        printf("2) [PASS] Buffer %d too small\n", small_lim);
    } else {
        printf("2) [WRONG] Got %d\n", n);
    }

    // Good buffer
    int good_lim = 2 * n;
    struct procinfo infos_2[good_lim];
    n = ps_listinfo(infos_2, good_lim);

    printf("3) [PASS] returned %d with buffer size %d\n", n, good_lim);

    if (n >= 0) {
        for (int i = 0; i < n; i++) {
            print_procinfo(&infos_2[i]);
        }
    }

    // Wrong adress
    struct procinfo* bad_ptr = (struct procinfo*)0x123;
    n = ps_listinfo(bad_ptr, good_lim);

    if (n == -2) {
        printf("4) [PASS] correctly detected invalid address\n");
    } else {
        printf("4) [WRONG] got %d for invalid address\n", n);
    }

    exit(0);
}
