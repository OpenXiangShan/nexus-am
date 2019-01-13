#include <am.h>
#include <mips32.h>

// Define this macro after serial has been implemented
#define HAS_SERIAL

#define SERIAL_PORT 0x43f8

extern char _heap_start;
extern char _heap_end;
extern int main();

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

void _putc(char ch) {
#ifdef HAS_SERIAL
  outb(SERIAL_PORT, ch);
#endif
}

void _halt(int code) {
  __asm__ volatile("move $v0, %0; .word 0xf0000000" : :"r"(code));

  // should not reach here
  while (1);
}

void _trm_init() {
  int ret = main();
  _halt(ret);
}
