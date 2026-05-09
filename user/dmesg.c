#include "kernel/types.h"
#include "user/user.h"

#define BUF_SIZE 4096 * 2

int main(void) {
    char* buf = malloc(BUF_SIZE);

    if(buf == 0 || dmesg(buf, BUF_SIZE) < 0){
        printf("dmesg failed\n");
        exit(1);
    }

    printf("%s", buf);
    exit(0);
}
