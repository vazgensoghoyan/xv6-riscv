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
    struct spinlock seed_lock;
    uint64 seed;

    struct spinlock total_written_lock;
    uint64 total_written;

} pseudo;

uint32 lcg_rand(void) {
    acquire(&pseudo.seed_lock);
    pseudo.seed = (1664525 * pseudo.seed + 1013904223);
    release(&pseudo.seed_lock);
    return (uint32)(pseudo.seed >> 32);
}

int pseudoread(int minor, int user_dst, uint64 addr, int n) {

    int buf_size = 64;
    char buf[buf_size];
    int i, j, chunk;

    switch (minor) {
        case PSEUDO_NULL:
            return 0;

        case PSEUDO_ZERO:
            memset(buf, 0, buf_size);
            for (i = 0; i < n; i += j) {
                j = (n - i < buf_size) ? n - i : buf_size;
                if (either_copyout(user_dst, addr + i, buf, j) < 0)
                    return -1;
            }
            return n;

        case PSEUDO_URANDOM:
            for (i = 0; i < n; i += chunk) {
                chunk = (n - i < buf_size) ? (n - i) : buf_size;
                
                for (j = 0; j < chunk; j++)
                    buf[j] = lcg_rand() & 0xFF;

                if (either_copyout(user_dst, addr + i, buf, chunk) < 0)
                    return -1;
            }
            return n;

        case PSEUDO_NULLSTAT:
            if (n != sizeof(uint64))
                return -1;

            acquire(&pseudo.total_written_lock);
            uint64 val = pseudo.total_written;
            release(&pseudo.total_written_lock);

            if (either_copyout(user_dst, addr, (char*)&val, sizeof(uint64)) < 0)
                return -1;

            return sizeof(uint64);
    }
    return -1;
}

int pseudowrite(int minor, int user_src, uint64 addr, int n) {

    switch(minor) {
        case PSEUDO_NULL:
            return n;

        case PSEUDO_ZERO:
            return -1;

        case PSEUDO_URANDOM:
            if (n != sizeof(uint64))
                return -1;

            uint64 new_seed;
            if (either_copyin((char*)&new_seed, user_src, addr, sizeof(uint64)) < 0)
                return -1;

            acquire(&pseudo.seed_lock);
            pseudo.seed = new_seed;
            release(&pseudo.seed_lock);

            return n;

        case PSEUDO_NULLSTAT:
            acquire(&pseudo.total_written_lock);
            pseudo.total_written += n;
            release(&pseudo.total_written_lock);
            return n;
    }
    return -1;
}

void pseudoinit(void) {
    initlock(&pseudo.seed_lock, "pseudo_seed_lock");
    initlock(&pseudo.total_written_lock, "pseudo_total_written_lock");

    acquire(&pseudo.seed_lock);
    pseudo.seed = 1;
    release(&pseudo.seed_lock);
    
    acquire(&pseudo.total_written_lock);
    pseudo.total_written = 0;
    release(&pseudo.total_written_lock);

    devsw[PSEUDO].read = pseudoread;
    devsw[PSEUDO].write = pseudowrite;
}
