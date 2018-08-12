#include <am-x86.h>

_Area _heap; // the heap memory defined in AM spec

int main();
static void heap_init();

// the bootloader jumps here,
//   with a (small) bootstrap stack
void _start() {
  // setup a C runtime environment
  bootcpu_init();
  heap_init();
  percpu_initgdt();
  percpu_initlapic();
  ioapic_init();

  int ret = main();
  _halt(ret);
}

void _putc(char ch) {
  // works for qemu, but not real hardware
  #define COM1_PORT 0x3f8
  outb(COM1_PORT, ch);
}

void _halt(int code) {
  cli();
  allcpu_halt();
  char buf[] = "Exited (#).\n";
  for (char *p = buf; *p; p++) {
    _putc((*p == '#') ? ('0' + code) : *p);
  }
  thiscpu_die();
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
