#include <am.h>
#include <x86.h>
#include "x86_64-qemu.h"

_Area _heap = {}; // the heap memory defined in AM spec

int main(const char *args);
static void heap_init();

void _start_c(char *args) {
  // TODO: APIC & multicore boot
  heap_init();
  int ret = main(args);
  _halt(ret);
}

void _putc(char ch) {
  #define COM1 0x3f8
  outb(COM1, ch);
}

void _halt(int code) {
  while (1);
}

#include <klib.h>

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
