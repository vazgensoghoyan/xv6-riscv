#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/defs.h"

static uint64 seed = 1;
static uint64 total_written = 0;

static uint32 lcg_rand(void) {
    seed = (1664525 * seed + 1013904223);
    return (uint32)(seed >> 32);
}

int pseudoread(int minor, int user_dst, uint64 addr, int n) {

    char buf[64];

    switch(minor){
        case PSEUDO_NULL:
            return 0;

        case PSEUDO_ZERO:
            for(int i = 0; i < n; i++){
                buf[0] = 0;
                if(either_copyout(user_dst, addr + i, buf, 1) < 0)
                    return -1;
            }
            return n;

        case PSEUDO_URANDOM:
            for(int i = 0; i < n; i++){
                buf[0] = lcg_rand() & 0xFF;
                if(either_copyout(user_dst, addr + i, buf, 1) < 0)
                    return -1;
            }
            return n;

        case PSEUDO_NULLSTAT:
            if(n != sizeof(uint64))
                return -1;
            if(either_copyout(user_dst, addr, (char*)&total_written, sizeof(uint64)) < 0)
                return -1;
            return sizeof(uint64);
    }
    return -1;
}

int pseudowrite(int minor, int user_src, uint64 addr, int n) {

    switch(minor){
        case PSEUDO_NULL:
            return n;

        case PSEUDO_ZERO:
            return -1;

        case PSEUDO_URANDOM:
            if(n != sizeof(uint64))
                return -1;
            uint64 new_seed;
            if(either_copyin((char*)&new_seed, user_src, addr, sizeof(uint64)) < 0)
                return -1;
            seed = new_seed;
            return n;

        case PSEUDO_NULLSTAT:
            total_written += n;
            return n;
    }
    return -1;
}

void pseudoinit(void) {
    devsw[PSEUDO].read = pseudoread;
    devsw[PSEUDO].write = pseudowrite;
}
