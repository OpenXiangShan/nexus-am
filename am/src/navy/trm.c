#include <am.h>
#include <stdio.h>
#include <stdlib.h>
#include <klib-macros.h>

#define HEAP_SIZE (8 * 1024 * 1024)
static uint8_t heap[HEAP_SIZE] = {};
_Area _heap = RANGE(heap, heap + HEAP_SIZE);

void _trm_init() {
}

void _putc(char ch) {
  putchar(ch);
}

void _halt(int code) {
  printf("Exit (%d)\n", code);
  exit(code);
}
