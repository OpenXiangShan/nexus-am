#include <nemu.h>

void _putc(char ch) {
  outb(0xa10003f8, ch);
}

void _halt(int code) {
  asm volatile ("move $v0, %0; .word 0xf0000000" : :"r"(code));

  // should not reach here
  while (1);
}
