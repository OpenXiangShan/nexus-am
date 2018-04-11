#include <x86.h>
#include <am.h>
#include <am-x86.h>

int main();

_Area _heap;

// jump to main()

void _trm_init();

void _start() {
  _trm_init();
  int ret = main();
  _halt(ret);
}

#define SERIAL_PORT 0x3f8

static void serial_init() {
  outb(SERIAL_PORT + 1, 0x00);
  outb(SERIAL_PORT + 3, 0x80);
  outb(SERIAL_PORT + 0, 0x01);
  outb(SERIAL_PORT + 1, 0x00);
  outb(SERIAL_PORT + 3, 0x03);
  outb(SERIAL_PORT + 2, 0xC7);
  outb(SERIAL_PORT + 4, 0x0B);
}

static void memory_init() {
  extern char end;
  unsigned long st, ed;
  unsigned long step = 1L<<20; // 1 MB step
  st = ed = (((unsigned long)&end) & ~(step-1)) + step;
  while (1) {
    volatile int *ptr = (int*)ed;
    *ptr = 0x5a5a5a5a;
    if (*ptr == 0x5a5a5a5a) {
      ed += step;
    } else {
      break;
    }
  }
  _heap.start = (void*)st;
  _heap.end = (void*)ed;
}

void _trm_init() {
  serial_init();
  memory_init();
  ioapic_init();
}

void _putc(char ch) {
  while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
  outb(SERIAL_PORT, ch);
}

static void puts(const char *s) {
  for (const char *p = s; *p; p ++) {
    _putc(*p);
  }
}

void _halt(int code) {
  puts("Exited (");
  if (code == 0) _putc('0');
  else {
    char buf[10], *p = buf + 9;
    for (*p = 0; code; code /= 10) {
      *(-- p) = '0' + code % 10;
    }
    puts(p);
  }
  puts(").\n");
  asm volatile("cli; hlt");
}
