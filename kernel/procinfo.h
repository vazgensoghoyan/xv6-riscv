// this enam is for user space

enum u_procstate { U_UNUSED, U_USED, U_SLEEPING, U_RUNNABLE, U_RUNNING, U_ZOMBIE };

// info about procces which we can give to user space

struct procinfo {
    // as I think, we dont need spinlocks, because this will not be global

    int pid;                     // Process ID
    int parent_pid;              // Parent process ID

    enum u_procstate state;      // Process state
    
    char name[16];               // Process name (debugging)
};
