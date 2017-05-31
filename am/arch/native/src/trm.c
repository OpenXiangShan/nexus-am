#include <am.h>
#include <stdio.h>
#include <stdlib.h>

#define HEAP_SIZE (64 * 1024 * 1024)

static char heap[HEAP_SIZE];

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
  printf("Exit (%d)\n", code);
  _exit(code);
}

_Area _heap = {
  .start = heap,
  .end = heap + HEAP_SIZE,
};
