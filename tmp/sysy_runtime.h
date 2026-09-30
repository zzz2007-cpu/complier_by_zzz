#ifndef LOCAL_SYSY_RUNTIME_H
#define LOCAL_SYSY_RUNTIME_H

#include <stdio.h>

int get_int(void);
char get_char(void);
void get_string(char *buffer, int max_len);
void put_int(int value);
void put_char(char value);
void put_string(char *text);

#endif
