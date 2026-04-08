#include "types.h"
#include "param.h"
#include "riscv.h"
#include "memlayout.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"
#include "vm.h"

static void print_flags(pte_t pte, char *out) {
    out[0] = (pte & PTE_R) ? 'R' : '_';
    out[1] = (pte & PTE_W) ? 'W' : '_';
    out[2] = (pte & PTE_X) ? 'X' : '_';
    out[3] = (pte & PTE_U) ? 'U' : '_';
    out[4] = '_'; // G нам не надо
    out[5] = (pte & PTE_A) ? 'A' : '_';
    out[6] = (pte & PTE_D) ? 'D' : '_';
    out[7] = '\0';
}

static void walk_pt(pagetable_t pagetable, int level, uint64 va) {
    for (int i = 0; i < 512; i++){
        pte_t pte = pagetable[i];

        if ((pte & PTE_V) == 0) continue;

        uint64 pa = PTE2PA(pte);
        int is_leaf = pte & (PTE_R | PTE_W | PTE_X);

        if (level > 0 && !is_leaf){
            uint64 new_va = va | ((uint64)i << PXSHIFT(level));
            walk_pt((pagetable_t)pa, level - 1, new_va);
        } else {
            uint64 vpn = va | ((uint64)i << PXSHIFT(level));

            char flags[8];
            print_flags(pte, flags);

            printf("0x%lx -> 0x%lx %s\n", vpn, pa, flags);
        }
    }
}

uint64 sys_pgtbl(void) {
    struct proc *p = myproc();
    printf("PAGETABLE %p\n", p->pagetable);
    walk_pt(p->pagetable, 2, 0);
    return 0;
}
