#include "x86-qemu.h"

_Area _heap = {};
volatile uint32_t *__am_lapic;
int __am_ncpu = 0;
struct cpu_local __am_cpuinfo[MAX_CPU];

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

_Area __am_heap_init() {
  extern char end;
  outb(0x70, 0x34);
  uint32_t lo = inb(0x71);
  outb(0x70, 0x35);
  uint32_t hi = inb(0x71) + 1;
  return RANGE(ROUNDUP(&end, 1 << 20), (uintptr_t)((lo | hi << 8) << 16));
}

void __am_lapic_init() {
  for (char *st = (char *)0xf0000; st != (char *)0xffffff; st ++) {
    if (*(volatile uint32_t *)st == 0x5f504d5f) {
      uint32_t mpconf_ptr = ((volatile MPDesc *)st)->conf;
      MPConf *conf = upcast(mpconf_ptr);
      __am_lapic = upcast(conf->lapicaddr);
      for (volatile char *ptr = (char *)(conf + 1);
           ptr < (char *)conf + conf->length; ptr += 8) {
        if (*ptr == '\0') {
          ptr += 12;
          panic_on(++__am_ncpu > MAX_CPU, "cannot support > MAX_CPU processors");
        }
      }
      return;
    }
  }
  bug();
}

void __am_percpu_initgdt() {
#if __x86_64__
  SegDesc *gdt = CPU->gdt;
  TSS64 *tss = &CPU->tss;
  gdt[SEG_KCODE] = SEG64(STA_X | STA_R,                      DPL_KERN);
  gdt[SEG_KDATA] = SEG64(STA_W,                              DPL_KERN);
  gdt[SEG_UCODE] = SEG64(STA_X | STA_R,                      DPL_USER);
  gdt[SEG_UDATA] = SEG64(STA_W,                              DPL_USER);
  gdt[SEG_TSS]   = SEG16(STS_T32A,      tss, sizeof(*tss)-1, DPL_KERN);
  bug_on((uintptr_t)tss >> 32);
  set_gdt(gdt, sizeof(gdt[0]) * (NR_SEG + 1));
  set_tr(KSEL(SEG_TSS));
#else
  SegDesc *gdt = CPU->gdt;
  TSS32 *tss = &CPU->tss;
  gdt[SEG_KCODE] = SEG32(STA_X | STA_R,   0,     0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG32(STA_W,           0,     0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG32(STA_X | STA_R,   0,     0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG32(STA_W,           0,     0xffffffff, DPL_USER);
  gdt[SEG_TSS]   = SEG16(STS_T32A,      tss, sizeof(*tss)-1, DPL_KERN);
  set_gdt(gdt, sizeof(gdt[0]) * NR_SEG);
  set_tr(KSEL(SEG_TSS));
#endif
}
