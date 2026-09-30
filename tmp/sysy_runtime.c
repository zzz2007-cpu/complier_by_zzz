#include "sysy_runtime.h"

#include <stdlib.h>
#include <string.h>

int get_int(void) {
    char line[128];
    if (fgets(line, sizeof(line), stdin) == NULL) {
        return 0;
    }
    return (int) strtol(line, NULL, 10);
}

char get_char(void) {
    int value = getchar();
    return value == EOF ? 0 : (char) value;
}

void get_string(char *buffer, int max_len) {
    if (max_len <= 0) {
        return;
    }
    if (fgets(buffer, max_len, stdin) == NULL) {
        buffer[0] = '\0';
    }
}

void put_int(int value) {
    printf("%d", value);
}

void put_char(char value) {
    putchar((unsigned char) value);
}

void put_string(char *text) {
    fputs(text, stdout);
}
