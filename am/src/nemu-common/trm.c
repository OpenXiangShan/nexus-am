#include <am.h>
#include <nemu.h>

extern char _heap_start;
extern char _heap_end;
int main(const char *args);

_Area _heap = RANGE(&_heap_start, &_heap_end);

void _putc(char ch) {
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  nemu_trap(code);

  // should not reach here
  while (1);
}

void _trm_init() {
  extern const char __am_mainargs;
  int ret = main(&__am_mainargs);
  _halt(ret);
}
