#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int g_var1 = 10;
int g_var2 = 20;

int main() {
    printf("=== START STATE (GLOBAL + STACK) ===\n");
    pgtbl();

    int s_var1 = 100;
    int s_var2 = 200;

    printf("\n=== AFTER STACK VARIABLES CREATED ===\n");
    printf("stack vars: %d %d\n", s_var1, s_var2);
    pgtbl();

    int s_arr[1024]; // ? 4KB

    s_arr[0] = 1;
    s_arr[1023] = 2;

    printf("\n=== AFTER STACK ARRAY ACCESS ===\n");
    printf("stack array: %d %d\n", s_arr[0], s_arr[1023]);
    pgtbl();

    printf("\n=== MALLOC HEAP (MULTIPLE PAGES) ===\n");
    char *buf = malloc(PGSIZE * 3);

    if (buf == 0){
        printf("malloc failed\n");
        exit(1);
    }

    printf("heap buffer: %p\n", buf);

    pgtbl();

    printf("\n=== CLEAR A/D FLAGS ===\n");
    pteflags_clear(buf, PGSIZE * 3, PTE_A | PTE_D);

    pgtbl();

    // чтение (должен выставиться A)
    printf("\n=== READ HEAP PAGES ===\n");
    volatile char r1 = buf[0];
    volatile char r2 = buf[PGSIZE];
    volatile char r3 = buf[PGSIZE * 2];

    printf("read: %d %d %d\n", r1, r2, r3);

    pgtbl();

    // запись (должен выставиться D)
    printf("\n=== WRITE HEAP PAGES ===\n");
    buf[0] = 1;
    buf[PGSIZE] = 2;
    buf[PGSIZE * 2] = 3;

    pgtbl();

    // повторная очистка
    printf("\n=== CLEAR A/D AGAIN ===\n");
    pteflags_clear(buf, PGSIZE * 3, PTE_A | PTE_D);

    pgtbl();

    printf("\n=== CHECK FLAGS ===\n");
    int has_flags = pteflags_check(buf, PGSIZE * 3, PTE_A | PTE_D);
    printf("A/D present: %d\n", has_flags);

    printf("\n=== FREE HEAP ===\n");
    free(buf);

    pgtbl();

    printf("\n=== DONE ===\n");
    exit(0);
}
