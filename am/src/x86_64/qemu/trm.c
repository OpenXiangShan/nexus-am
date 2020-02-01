#include "x86_64-qemu.h"

_Area _heap = {}; // the heap memory defined in AM spec

int main(const char *args);

static void call_main(const char *args) {
  _halt(main(args));
}

void _start_c(char *args) {
  if (boot_record()->is_ap) {
    __am_othercpu_entry();
  } else {
    __am_bootcpu_init();
    stack_switch_call(stack_top(&CPU->stack), call_main, (uintptr_t)args);
  }
}

void __am_bootcpu_init() {
  _heap = __am_heap_init();
  __am_lapic_init();
  __am_ioapic_init();
  __am_percpu_init();
}

void __am_percpu_init() {
  __am_percpu_initgdt();
  __am_percpu_initlapic();
  __am_percpu_initirq();
}

void _putc(char ch) {
  #define COM1 0x3f8
  outb(COM1, ch);
}

void _halt(int code) {
  const char *hex = "0123456789abcdef";
  const char *fmt = "CPU #$ Halt (40).\n";
  cli();
  __am_stop_the_world();
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
  outw(0x604, 0x2000); // offer of qemu :)
  while (1) hlt();
}
