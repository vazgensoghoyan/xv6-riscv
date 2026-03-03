#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int is_digit(char c) {
    return '0' <= c && c <= '9';
}

int check_format(const char* str) {
    int i = 0;
    while (is_digit(str[i])) {
        i++;
    }
    if (i == 0 || str[i] != ' ') return 0;
    i++;
    if (!is_digit(str[i])) return 0;
    while (is_digit(str[i])) {
        i++;
    }
    return str[i] == '\0';
}

int main(int argc, char* argv[]) {
    int MAX_SIZE = 100;
    char buf[MAX_SIZE];

    int i, read_count;
    char char_in;

    for(i = 0; i + 1 < MAX_SIZE; i++) {
        read_count = read(0, &char_in, 1);

        if (read_count < 0) {
            fprintf(2, "Error reading input\n");
            exit(1);
        }
        if (read_count == 0) break; // eof

        buf[i] = char_in;
        if (char_in == '\n') break;
    }
    buf[i] = '\0';

    if (!check_format(buf)) {
        fprintf(2, "Invalid input format\n");
        exit(1);
    }

    printf("|%s|\n", buf); // логируем по заданию

    char * fst_p = buf;
    char* snd_p = buf;
    
    // уже имеем корректный формат
    while (*snd_p != ' ') snd_p++;
    *snd_p = '\0'; // разделяем числа
    snd_p++;

    int a = atoi(fst_p);
    int b = atoi(snd_p);

    int sum = add(a, b); // системный вызов

    printf("%d\n", sum);

    exit(0);
}
