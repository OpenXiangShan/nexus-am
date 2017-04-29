#include <am.h>
#include <stdio.h>
#include <stdlib.h>

#define HEAP_SIZE (128 * 1024 * 1024)

void _trm_init() {
  char *heap = (char*)malloc(HEAP_SIZE);
  if (heap) {
    _heap.start = heap;
    _heap.end = heap + HEAP_SIZE;
  }
}

void _putc(char ch) {
  putchar(ch);
}

void _halt(int code) {
  _exit(code);
}

_Area _heap;
