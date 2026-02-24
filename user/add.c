#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]) {
    int MAX_SIZE = 100;
    char buf[MAX_SIZE];

    int i, read_count;
    char char_in;

    for(i = 0; i + 1 < MAX_SIZE; i++) {
        read_count = read(0, &char_in, 1);

        if (read_count < 0) {
            write(2, "Error reading input\n", 20);
            exit(1);
        }
        if(read_count == 0) break; // EOF

        buf[i] = char_in;
        if(char_in == '\n') break;
    }
    buf[i] = '\0';

    printf("|%s|\n", buf);

    char * fst_p = buf;
    char* snd_p = buf;
    
    while (*snd_p && *snd_p != ' ') snd_p++;
    
    if (*snd_p == '\0') {
        write(2, "Expected two integers separated by space\n", 42);
        exit(1);
    }

    *snd_p = '\0'; // разделяем числа
    snd_p++;

    int a = atoi(fst_p);
    int b = atoi(snd_p);

    int sum = add(a, b); // системный вызов

    printf("%d\n", sum);

    exit(0);
}
