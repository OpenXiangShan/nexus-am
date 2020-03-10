#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <nemu.h>

extern char _heap_start;
int main(const char *args);

_Area _heap = RANGE(&_heap_start, &_pmem_end);

void _trm_init() {
  extern const char __am_mainargs;
  int ret = main(&__am_mainargs);
  _halt(ret);
}

void _putc(char ch) {
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  printf("Exit with code = %d\n", code);
  printf("Spinning\n");
  while (1);
}
