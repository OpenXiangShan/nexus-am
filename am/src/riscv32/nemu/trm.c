#include <am.h>
#include <nemu.h>

extern char _heap_start;
extern char _heap_end;
int main();

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

void _putc(char ch) {
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  __asm__ volatile("mv a0, %0; .word 0x0000006b" : :"r"(code));

  // should not reach here
  while (1);
}

void _trm_init() {
  int ret = main();
  _halt(ret);
}
