#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

struct {
    struct spinlock lock;

    uint64 seed;
    uint64 total_written;

} pseudo;

uint32 lcg_rand(void) {        // на каждый байт lock, неэффективно, но ничего
    acquire(&pseudo.lock);
    pseudo.seed = (1664525 * pseudo.seed + 1013904223);
    uint32 result = (uint32)(pseudo.seed >> 32);
    release(&pseudo.lock);
    return result;
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

            acquire(&pseudo.lock);
            uint64 val = pseudo.total_written;
            release(&pseudo.lock);

            if(either_copyout(user_dst, addr, (char*)&val, sizeof(uint64)) < 0)
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

            acquire(&pseudo.lock);
            pseudo.seed = new_seed;
            release(&pseudo.lock);

            return n;

        case PSEUDO_NULLSTAT:
            acquire(&pseudo.lock);
            pseudo.total_written += n;
            release(&pseudo.lock);
            return n;
    }
    return -1;
}

void pseudoinit(void) {
    initlock(&pseudo.lock, "pseudo");

    acquire(&pseudo.lock);
    pseudo.seed = 1;
    pseudo.total_written = 0;
    release(&pseudo.lock);

    devsw[PSEUDO].read = pseudoread;
    devsw[PSEUDO].write = pseudowrite;
}
