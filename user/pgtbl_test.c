#include "kernel/types.h"
#include "kernel/riscv.h"
#include "user/user.h"

#define PGSIZE 4096

// Глобальная переменная
int global_var = 42;

void
print_header(char *msg)
{
  printf("\n============================\n");
  printf("%s\n", msg);
  printf("============================\n");
}

// Буфер для проверки на куче
void
test_heap()
{
  print_header("ALLOCATING HEAP (MULTIPLE PAGES)");

  char *heap = malloc(5 * PGSIZE); // несколько страниц

  if(heap == 0){
    printf("malloc failed\n");
    exit(1);
  }

  // Инициализация массива
  for(int i = 0; i < 5 * PGSIZE; i++){
    heap[i] = (char)(i % 256);
  }

  printf("Heap allocated and initialized\n");

  pgtbl();

  print_header("CLEAR A/D FLAGS ON HEAP");

  // Очистка флагов
  pteflags_clear(heap, 5 * PGSIZE, PTE_A | PTE_D);

  pgtbl();

  print_header("READ FROM HEAP (SHOULD SET A)");

  volatile char x = heap[0]; // чтение
  printf("Read value: %d\n", x);

  pgtbl();

  print_header("WRITE TO HEAP (SHOULD SET D)");

  heap[0] = 99;

  pgtbl();

  print_header("CHECK FLAGS ON HEAP");

  int res = pteflags_check(heap, 5 * PGSIZE, PTE_A | PTE_D);
  printf("Check A|D result: %d (expected 1)\n", res);

  print_header("FREE HEAP");

  free(heap);

  pgtbl();
}

void
test_stack()
{
  print_header("STACK TEST");

  int stack_var = 123;
  int stack_arr[1024]; // лежит в стеке

  for(int i = 0; i < 1024; i++){
    stack_arr[i] = i;
  }

  printf("Stack var address: %p\n", &stack_var);
  printf("Stack array address: %p\n", stack_arr);

  pgtbl();

  print_header("CLEAR FLAGS ON STACK");

  pteflags_clear(&stack_var, sizeof(stack_var), PTE_A | PTE_D);

  pgtbl();

  print_header("READ STACK");

  volatile int x = stack_var;
  printf("Read stack: %d\n", x);

  pgtbl();

  print_header("WRITE STACK");

  stack_var = 777;

  pgtbl();
}

void
test_global()
{
  print_header("GLOBAL VARIABLE TEST");

  printf("Global var address: %p\n", &global_var);

  pgtbl();

  print_header("READ GLOBAL");

  volatile int x = global_var;
  printf("Global read: %d\n", x);

  pgtbl();

  print_header("WRITE GLOBAL");

  global_var = 100;

  pgtbl();
}

int
main()
{
  print_header("START");

  pgtbl();

  test_global();
  test_stack();
  test_heap();

  print_header("END");

  exit(0);
}