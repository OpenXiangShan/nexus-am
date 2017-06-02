#include <am.h>
#include <mips-qemu.h>

extern char _end;
extern int main();

_Area _heap = {
  .start = &_end,
  .end   = &_end,
};

_Screen _screen = {
  .width  = 0,
  .height = 0,
};

static void serial_init() {
  SERIAL_PORT[1] = 0x00;
  SERIAL_PORT[3] = 0x80;
  SERIAL_PORT[0] = 0x01;
  SERIAL_PORT[1] = 0x00;
  SERIAL_PORT[3] = 0x03;
  SERIAL_PORT[2] = 0xc7;
  SERIAL_PORT[4] = 0x0b;
}

void _putc(char ch) {
  while ((SERIAL_PORT[5] & 0x20) == 0);
  SERIAL_PORT[0] = ch;
}

void _halt(int code) {
  for (const char *p = "Halt\n"; *p; p ++) {
    _putc(*p);
  }
  while (1);
}

void _trm_init() {
  serial_init();
  int ret = main();
  _halt(ret);
}
