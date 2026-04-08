#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int print_hex(uint8 b) {
    const char* hex = "0123456789ABCDEF";

    if (write(1, &hex[b >> 4], 1) != 1)
        return -1;

    if (write(1, &hex[b & 0xF], 1) != 1)
        return -1;

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(2, "Usage: hexdump <nbytes> <file>\n");
        exit(1);
    }

    int n = atoi(argv[1]);
    char* file = argv[2];

    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        fprintf(2, "hexdump: cannot open %s\n", file);
        exit(1);
    }

    uint8 buf[512];
    int total = 0;

    while (total < n) {
        int to_read = sizeof(buf);
        if (n - total < to_read)
            to_read = n - total;

        int r = read(fd, buf, to_read);

        if (r < 0) {
            fprintf(2, "hexdump: read error\n");
            close(fd);
            exit(1);
        }

        if (r == 0) {
            break; // EOF
        }

        for (int i = 0; i < r; i++) {
            if (print_hex(buf[i]) < 0) {
                fprintf(2, "hexdump: write error\n");
                close(fd);
                exit(1);
            }

            if (total + i < n - 1) {
                if (write(1, " ", 1) != 1) {
                    fprintf(2, "hexdump: write error\n");
                    close(fd);
                    exit(1);
                }
            }
        }

        total += r;
    }

    if (write(1, "\n", 1) != 1) {
        fprintf(2, "hexdump: write error\n");
        close(fd);
        exit(1);
    }

    close(fd);
    exit(0);
}
