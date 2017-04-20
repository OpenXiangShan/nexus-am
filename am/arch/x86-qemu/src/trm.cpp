#include <x86.h>
#include <am.h>


extern "C" {

int main();

_Area _heap;

// jump to main()

void _start() {
  int ret = main();
  _halt(ret);
}

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
  while (true) {
    volatile int *ptr = (int*)ed;
    *ptr = 0x5a5a5a5a;
    if (*ptr == 0x5a5a5a5a) {
      ed += step;
    } else {
      break;
    }
  }
  _heap.start = reinterpret_cast<void*>(st);
  _heap.end = reinterpret_cast<void*>(ed);
}

void _trm_init() {
  serial_init();
  memory_init();
}

void _putc(char ch) {
  while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
  outb(SERIAL_PORT, ch);
}

void _halt(int code) {
  _putc('P'); _putc('a'); _putc('n'); _putc('i'); _putc('c'); _putc('\n');
  asm volatile("cli; hlt");
}


