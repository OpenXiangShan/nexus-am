#include <nemu.h>

void _putc(char ch) {
  outb(0x3f8, ch);
}

void _halt(int code) {
  asm volatile (".byte 0xd6" : :"a"(code));

  // should not reach here
  while (1);
}
