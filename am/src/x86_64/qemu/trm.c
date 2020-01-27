#include <am.h>
#include "x86_64-qemu.h"

_Area _heap = {}; // the heap memory defined in AM spec

int main(const char *args);

void _start_c(char *args) {
  if (boot_record()->is_ap) {
    othercpu_entry();
  } else {
    bootcpu_init();
    __am_percpu_init();
    int ret = main(args);
    _halt(ret);
  }
}

void _putc(char ch) {
  #define COM1 0x3f8
  outb(COM1, ch);
}

void _halt(int code) {
  const char *hex = "0123456789abcdef";
  const char *fmt = "CPU #$ Halt (40).\n";
  cli();
  __am_othercpu_halt();
  for (const char *p = fmt; *p; p++) {
    char ch = *p;
    switch (ch) {
      case '$':
        _putc(hex[_cpu()]);
        break;
      case '0': case '4':
        _putc(hex[(code >> (ch - '0')) & 0xf]);
        break;
      default:
        _putc(ch);
    }
  }
  __am_thiscpu_halt();
}
