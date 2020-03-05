#include <nemu.h>

void _putc(char ch) {
  outb(0xa10003f8, ch);
}

void _halt(int code) {
  asm volatile("mv a0, %0; .word 0x0000006b" : :"r"(code));

  // should not reach here
  while (1);
}
