#include "x86_64-qemu.h"

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
  int32_t magic = 0x5a5aa5a5;
  int32_t step = 1L << 20;
  extern char end;
  uintptr_t st, ed;
  for (st = ed = ROUNDUP(&end, step); ; ed += step) {
    volatile uint32_t *ptr = (uint32_t *)ed;
    if ((*ptr = magic, *ptr) != magic) {
      break; // read-after-write fail
    }
  }
  return RANGE(st, ed);
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
  panic("seems not an x86-qemu virtual machine");
}

void __am_percpu_initgdt() {
#if __x86_64__
  SegDesc64 *gdt = CPU->gdt;
  uint64_t tss = (uint64_t)(&CPU->tss);
  gdt[0]         = 0;
  gdt[SEG_KCODE] = 0x0020980000000000LL;
  gdt[SEG_KDATA] = 0x0000920000000000LL;
  gdt[SEG_UCODE] = 0x0020F80000000000LL;
  gdt[SEG_UDATA] = 0x0000F20000000000LL;
  gdt[SEG_TSS+0] = (sizeof(CPU->tss) - 1) | ((tss & 0xffffff) << 16) |
                   (0x00e9LL << 40) | (((tss >> 24) & 0xff) << 56);
  gdt[SEG_TSS+1] = (tss >> 32);
  set_gdt(gdt, sizeof(gdt[0]) * (NR_SEG + 1));
  set_tr(KSEL(SEG_TSS));
#else
  SegDesc32 *gdt = CPU->gdt;
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
