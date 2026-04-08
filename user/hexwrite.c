#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: hexwrite <hex_string> <file>\n");
        exit(1);
    }

    char* hex = argv[1];
    char* file = argv[2];

    int len = strlen(hex);

    if (len % 2 != 0) {
        fprintf(2, "hexwrite: hex string must have even length\n");
        exit(1);
    }

    int fd = open(file, O_WRONLY);
    if (fd < 0) {
        fprintf(2, "hexwrite: cannot open %s\n", file);
        exit(1);
    }

    int out_len = len / 2;
    uint8 buf[256];

    for (int i = 0; i < out_len; i++) {
        int hi = hexval(hex[2*i]);
        int lo = hexval(hex[2*i + 1]);

        if (hi < 0 || lo < 0) {
            fprintf(2, "hexwrite: invalid hex character\n");
            close(fd);
            exit(1);
        }

        buf[i] = (hi << 4) | lo;
    }

    int w = write(fd, buf, out_len);
    if (w < 0) {
        fprintf(2, "hexwrite: write error\n");
        close(fd);
        exit(1);
    }

    close(fd);
    exit(0);
}
