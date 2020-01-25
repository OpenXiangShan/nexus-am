#include <am.h>
#include "x86_64-qemu.h"
#include <klib.h>

_Area _heap = {}; // the heap memory defined in AM spec

int main(const char *args);

volatile uint32_t *__am_lapic;
int __am_ncpu = 0;
struct cpu_local __am_cpuinfo[MAX_CPU];

void _start_c(char *args) {
  if (boot_record()->is_ap) {
    (boot_record()->entry)();
  }

  bootcpu_init();
  percpu_init();

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
