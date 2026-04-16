#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

int g_var = 42;
int g_arr[8] = {1,2,3,4,5,6,7,8};

static void check_all(void *buf, int len, char *label) {
    int a = pteflags_check(buf, len, PTE_A);
    int d = pteflags_check(buf, len, PTE_D);
    printf("    [check] %s : A=%d D=%d\n", label, a, d);
}

static void PRINT_SECTION_TITLE(char *title) {
    printf("\n========================================\n");
    printf("  %s\n", title);
    printf("========================================\n");
}

int main() {

    PRINT_SECTION_TITLE("1. ТАБЛИЦА СТРАНИЦ ПРИ СТАРТЕ");
    pgtbl();

    PRINT_SECTION_TITLE("2. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ (g_var) И ГЛОБ МАССИВ (g_arr)");

    printf("\n-- действие: чтение g_var --\n");
    pteflags_clear(&g_var, sizeof(g_var), PTE_A | PTE_D);
    check_all(&g_var, sizeof(g_var), "до чтения");
    volatile int r = g_var; (void)r;
    check_all(&g_var, sizeof(g_var), "после чтения  [ожидаем A=1 D=0]");

    printf("\n-- действие: запись g_var --\n");
    pteflags_clear(&g_var, sizeof(g_var), PTE_A | PTE_D);
    check_all(&g_var, sizeof(g_var), "до записи");
    g_var = 99;
    check_all(&g_var, sizeof(g_var), "после записи [ожидаем A=1 D=1]");

    printf("\n-- действие: clear g_var --\n");
    check_all(&g_var, sizeof(g_var), "до clear");
    pteflags_clear(&g_var, sizeof(g_var), PTE_A | PTE_D);
    check_all(&g_var, sizeof(g_var), "после clear [ожидаем A=0 D=0]");

    printf("\n-- действие: чтение g_arr[0] --\n");
    pteflags_clear(g_arr, sizeof(g_arr), PTE_A | PTE_D);
    check_all(g_arr, sizeof(g_arr), "до чтения");
    r = g_arr[0]; (void)r;
    check_all(g_arr, sizeof(g_arr), "после чтения  [ожидаем A=1 D=0]");

    printf("\n-- действие: запись g_arr[0] --\n");
    pteflags_clear(g_arr, sizeof(g_arr), PTE_A | PTE_D);
    check_all(g_arr, sizeof(g_arr), "до записи");
    g_arr[0] = 55;
    check_all(g_arr, sizeof(g_arr), "после записи [ожидаем A=1 D=1]");

    PRINT_SECTION_TITLE("3. СТЕКОВАЯ ПЕРЕМЕННАЯ И МАССИВ ИЗ СТЕКА");

    int s_var = 100;
    int s_arr[4];

    // Стековые переменные на одной странице с фреймами вызовов,
    // поэтому clear/check показывает реальное поведение только
    // для записи (D), а A сбросить не получится — любой вызов
    // функции снова трогает стековую страницу.

    printf("\n-- действие: запись стековой переменной (s_var = ...) --\n");
    printf("  ЗАМЕЧАНИЕ: стековая страница используется тут нами же\n");
    printf("  Так что флаг 'A' не сбрасывается :)\n");
    s_var = 200;
    check_all(&s_var, sizeof(s_var), "после записи  [ожидаем A=1 D=1]");
    pteflags_clear(&s_var, sizeof(s_var), PTE_A | PTE_D);
    check_all(&s_var, sizeof(s_var), "после clear  [A=1 потому check() сам использовал стек]");

    printf("\n-- действие: запись массива из стека --\n");
    s_arr[0] = 1;
    s_arr[3] = 2;
    check_all(s_arr, sizeof(s_arr), "после записи  [ожидаем A=1 D=1]");
    pteflags_clear(s_arr, sizeof(s_arr), PTE_A | PTE_D);
    check_all(s_arr, sizeof(s_arr), "после clear [A=1 из за вызова фрейма на той же странице]");

    PRINT_SECTION_TITLE("4. ВЫДЕЛЕНИЯ ПАМЯТИ В КУЧЕ (3 страницы)");

    char *buf = malloc(PGSIZE * 3);
    if (buf == 0) { printf("malloc failed\n"); exit(1); }
    printf("heap buf: %p\n", buf);

    // Трогаем каждую страницу чтобы lazy alloc отработал
    volatile char tmp;
    tmp = buf[0];          (void)tmp;
    tmp = buf[PGSIZE];     (void)tmp;
    tmp = buf[PGSIZE*2];   (void)tmp;

    printf("\n-- page table после malloc + first touch --\n");
    pgtbl();

    PRINT_SECTION_TITLE("5. СНЯТИЕ A И D У ВСЕХ СТРАНИЦ КУЧИ");

    check_all(buf,            PGSIZE, "page 0 до clear");
    check_all(buf+PGSIZE,     PGSIZE, "page 1 до clear");
    check_all(buf+PGSIZE*2,   PGSIZE, "page 2 до clear");
    pteflags_clear(buf, PGSIZE * 3, PTE_A | PTE_D);
    check_all(buf,            PGSIZE, "page 0 после clear [ожидаем A=0 D=0]");
    check_all(buf+PGSIZE,     PGSIZE, "page 1 после clear [ожидаем A=0 D=0]");
    check_all(buf+PGSIZE*2,   PGSIZE, "page 2 после clear [ожидаем A=0 D=0]");

    printf("\n-- page table после clear --\n");
    pgtbl();

    PRINT_SECTION_TITLE("6. ЧТЕНИЕ ДАННЫХ КУЧИ (ожидаем A=1 D=0)");

    printf("\n-- действие: чтение only page 1 --\n");
    check_all(buf,            PGSIZE, "page 0 до чтения");
    check_all(buf+PGSIZE,     PGSIZE, "page 1 до чтения");
    check_all(buf+PGSIZE*2,   PGSIZE, "page 2 до чтения");
    tmp = buf[PGSIZE]; (void)tmp;
    check_all(buf,            PGSIZE, "page 0 после чтения [ожидаем A=0 D=0]");
    check_all(buf+PGSIZE,     PGSIZE, "page 1 после чтения [ожидаем A=1 D=0]");
    check_all(buf+PGSIZE*2,   PGSIZE, "page 2 после чтения [ожидаем A=0 D=0]");

    printf("\n-- page table после чтения --\n");
    pgtbl();

    PRINT_SECTION_TITLE("7. ИЗМЕНЕНИЕ ДАННЫХ КУЧИ (ожидаем A=1 D=1)");

    pteflags_clear(buf, PGSIZE * 3, PTE_A | PTE_D);

    printf("\n-- действие: запись only page 2 --\n");
    check_all(buf,            PGSIZE, "page 0 до записи");
    check_all(buf+PGSIZE,     PGSIZE, "page 1 до записи");
    check_all(buf+PGSIZE*2,   PGSIZE, "page 2 до записи");
    buf[PGSIZE*2] = 7;
    check_all(buf,            PGSIZE, "page 0 после записи [ожидаем A=0 D=0]");
    check_all(buf+PGSIZE,     PGSIZE, "page 1 после записи [ожидаем A=0 D=0]");
    check_all(buf+PGSIZE*2,   PGSIZE, "page 2 после записи [ожидаем A=1 D=1]");

    printf("\n-- page table после записи --\n");
    pgtbl();

    PRINT_SECTION_TITLE("8. ОСВОБОЖДЕНИЕ ПАМЯТИ");

    printf("  ЗАМЕЧАНИЕ: видим, что страницы остались замапленными\n");
    printf("  можем предположить, что free() в xv6 не вызывает sbrk()\n");

    free(buf);
    printf("-- page table после free --\n");
    pgtbl();

    PRINT_SECTION_TITLE("DONE");
    exit(0);
}
