#include "../x86-qemu.h"

_Area _heap = {}; // the heap memory defined in AM spec

int main();
static void heap_init();

void _start() { // the bootloader jumps here
  __am_bootcpu_init();
  heap_init();
  __am_percpu_initgdt();
  __am_percpu_initlapic();
  __am_ioapic_init();
  _halt(main());
}

void _putc(char ch) { // only works for x86-qemu
  #define COM1 0x3f8
  outb(COM1, ch); // first -serial device
}

void _halt(int code) {
  cli();
  __am_othercpu_halt();
  char buf[] = "Exited (#).\n";
  for (char *p = buf; *p; p++) {
    _putc((*p == '#') ? ('0' + code) : *p);
  }
  __am_thiscpu_halt();
}

static void heap_init() {
  #define MAGIC 0x5a5aa5a5
  #define STEP  (1L << 20)
  extern char end;
  uintptr_t st, ed;
  for (st = ed = ROUNDUP(&end, STEP); ; ed += STEP) {
    volatile uint32_t *ptr = (uint32_t *)ed;
    if ((*ptr = MAGIC, *ptr) != MAGIC) {
      break; // read-after-write fail
    }
  }
  _heap = RANGE(st, ed);
}
