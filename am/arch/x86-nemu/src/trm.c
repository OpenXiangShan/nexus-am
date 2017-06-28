#include <am.h>
#include <x86-nemu.h>
#include <x86.h>

#ifdef HAS_DEVICE
# define SERIAL_PORT 0x3f8
#endif

extern char _heap_start;
extern char _heap_end;
extern int main();

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

static void serial_init() {
#ifdef HAS_DEVICE
  outb(SERIAL_PORT + 1, 0x00);
  outb(SERIAL_PORT + 3, 0x80);
  outb(SERIAL_PORT + 0, 0x01);
  outb(SERIAL_PORT + 1, 0x00);
  outb(SERIAL_PORT + 3, 0x03);
  outb(SERIAL_PORT + 2, 0xC7);
  outb(SERIAL_PORT + 4, 0x0B);
#endif
}

void _putc(char ch) {
#ifdef HAS_DEVICE
  while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
  outb(SERIAL_PORT, ch);
#else
  asm volatile(".byte 0xd6" : :"a"(2), "b"((uint8_t)ch));
#endif
}

void _halt(int code) {
  asm volatile(".byte 0xd6" : :"a"(code));

  // should not reach here
  while (1);
}

void _trm_init() {
  serial_init();
  int ret = main();
  _halt(ret);
}
