#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"

// (just repeated includes from pipe.c)

int mutexalloc(struct file **f) {
  struct sleeplock *lk;

  if ((*f = filealloc()) == 0)
    return -1;

  if ((lk = (struct sleeplock*)kalloc()) == 0) {
    fileclose(*f);
    return -1;
  }

  initsleeplock(lk, "mutex");

  (*f)->type = FD_MUTEX;
  (*f)->readable = 0;
  (*f)->writable = 0;
  (*f)->mutex = lk;

  return f;
}

int mutexclose(struct sleeplock *lk) {
  if(lk == 0)
    panic("mutexclose");

  kfree((char*)lk);
  return 0;
}
