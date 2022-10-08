#include <nemu.h>

void _putc(char ch) {
  outb(0xa10003f8, ch);
}

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

void _halt(int code) {
  tohost = (code << 1) | 1;

  while (1);
}
