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

static void print_indent(int level) {
    int dots = (3 - level) * 4; // подобрано под визуализацию
    for(int i = 0; i < dots; i++){
        printf(".");
    }
}

static void walk_pt(pagetable_t pagetable, int level) {
    for (int i = 0; i < 512; i++){
        pte_t pte = pagetable[i];

        if ((pte & PTE_V) == 0) continue;

        uint64 pa = PTE2PA(pte);
        int is_leaf = pte & (PTE_R | PTE_W | PTE_X);

        char flags[8];
        print_flags(pte, flags);

        if(level > 0 && !is_leaf){
            print_indent(level);
            printf("0x%x -> 0x%lx %s\n", i, pa, flags);

            walk_pt((pagetable_t)pa, level - 1);
        } else {
            print_indent(level);
            printf("0x%x -> 0x%lx %s\n", i, pa, flags);
        }
    }
}

uint64 sys_pgtbl(void) {
    struct proc *p = myproc();
    printf("PAGETABLE %p\n", p->pagetable);
    walk_pt(p->pagetable, 2);
    return 0;
}

uint64
sys_pteflags_clear(void)
{
    uint64 buf;
    int len;
    int mask;

    argaddr(0, &buf);
    argint(1, &len);
    argint(2, &mask);

    // проверка маски (только A и D допустимы)
    if(mask & ~(PTE_A | PTE_D))
        return -1;

    struct proc *p = myproc();

    // проверка что весь буфер в адресном пространстве
    if(buf >= p->sz || buf + len > p->sz)
        return -1;

    uint64 start = PGROUNDDOWN(buf);
    uint64 end = PGROUNDDOWN(buf + len - 1);

    for(uint64 va = start; va <= end; va += PGSIZE){
        pte_t *pte = walk(p->pagetable, va, 0);
        if(pte == 0 || (*pte & PTE_V) == 0)
            return -1;

        // снимаем флаги
        *pte &= ~mask;
    }

    return 0;
}

uint64
sys_pteflags_check(void)
{
    uint64 buf;
    int len;
    int mask;

    argaddr(0, &buf);
    argint(1, &len);
    argint(2, &mask);

    if(mask & ~(PTE_A | PTE_D))
        return -1;

    struct proc *p = myproc();

    if(buf >= p->sz || buf + len > p->sz)
        return -1;

    uint64 start = PGROUNDDOWN(buf);
    uint64 end = PGROUNDDOWN(buf + len - 1);

    for(uint64 va = start; va <= end; va += PGSIZE){
        pte_t *pte = walk(p->pagetable, va, 0);
        if(pte == 0 || (*pte & PTE_V) == 0)
            return -1;

        if((*pte & mask) != 0)
            return 1;
    }

    return 0;
}
