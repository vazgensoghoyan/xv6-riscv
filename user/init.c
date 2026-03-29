// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char *argv[] = { "sh", 0 };

static void open_pseudo(const char* name, int minor) {
  int fd = open(name, O_RDWR);
  if(fd < 0){
    mknod(name, PSEUDO, minor);
  } else {
    close(fd);
    // closing is important as i understood
    // because otherwise doing 'make qemu' few times
    // we will get init.c working with same data few times
    // and therefore this pseudo files will stay opened
    // therefore console will not work properly
  }
}

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }

  open_pseudo("null", PSEUDO_NULL);
  open_pseudo("zero", PSEUDO_ZERO);
  open_pseudo("urandom", PSEUDO_URANDOM);
  open_pseudo("nullstat", PSEUDO_NULLSTAT);

  dup(0);  // stdout
  dup(0);  // stderr

  for(;;){
    printf("init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf("init: fork failed\n");
      exit(1);
    }
    if(pid == 0){
      exec("sh", argv);
      printf("init: exec sh failed\n");
      exit(1);
    }

    for(;;){
      // this call to wait() returns if the shell exits,
      // or if a parentless process exits.
      wpid = wait((int *) 0);
      if(wpid == pid){
        // the shell exited; restart it.
        break;
      } else if(wpid < 0){
        printf("init: wait returned an error\n");
        exit(1);
      } else {
        // it was a parentless process; do nothing.
      }
    }
  }
}
