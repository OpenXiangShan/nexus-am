#include <am.h>
#include <stdio.h>
#include <stdlib.h>

void _trm_init() {
}

void _putc(char ch) {
  putchar(ch);
}

void _halt(int code) {
  printf("Exit (%d)\n", code);
  _exit(code);
}

_Area _heap;
