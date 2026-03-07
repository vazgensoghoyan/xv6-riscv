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
    fprintf(1, "PID: %d, Parent PID: %d, State: %s, Name: %s\n",
        info->pid, info->parent_pid, state_to_string(info->state), info->name);
}

int try_ps_listinfo_stack(int lim) {
    struct procinfo infos[lim];
    int count = ps_listinfo(infos, lim);

    if (count >= 0) {
        for (int i = 0; i < count; i++) {
            print_procinfo(&infos[i]);
        }
    }

    return count;
}

int main(int argc, char* argv[]) {
    int lim = 1;
    int count = -1;

    while (count == -1) {
        count = try_ps_listinfo_stack(lim);

        lim *= 2;
        if (lim > 1000) {
            fprintf(2, "too many processes\n");
            exit(1);
        }
    }

    if (count == -2) {
        fprintf(2, "invalid user buffer\n");
        exit(1);
    }

    exit(0);
}
