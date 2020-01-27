#include <am.h>
#include "x86_64-qemu.h"

struct cpu_local __am_cpuinfo[MAX_CPU] = {};

static void (* volatile user_entry)();
static volatile intptr_t apboot_done = 0;

static void ap_entry();

int _mpe_init(void (*entry)()) {
  user_entry = entry;
  stack_jump(stack_top(&CPU->stack), percpu_entry, 0);
  panic("bug: should not return");
  return 1;
}

int _ncpu() {
  return __am_ncpu;
}

int _cpu(void) {
  return __am_lapic[8] >> 24;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile ("lock xchg %0, %1":
    "+m"(*addr), "=a"(result) : "1"(newval) : "cc");
  return result;
}

void percpu_entry() {
  if (_cpu() == 0) { // bootstrap cpu, boot all aps
    boot_record()->jmp_code = 0x000bfde9; // (16-bit) jmp (0x7c00)
    for (int cpu = 1; cpu < __am_ncpu; cpu++) {
      boot_record()->is_ap = 1;
      __am_lapic_bootap(cpu, 0x7000);
      while (_atomic_xchg(&apboot_done, 0) != 1) {
        pause();
      }
    }
    user_entry();
  } else { // this is an ap
    stack_jump(stack_top(&CPU->stack), ap_entry, 0);
  }
}

static void ap_entry() {
  percpu_init();
  _atomic_xchg(&apboot_done, 1);
  user_entry();
}

void percpu_init() {
  __am_percpu_initgdt();
  __am_percpu_initlapic();
  __am_percpu_initirq();
//  __am_percpu_initpg();

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
  gdt[SEG_TSS+0] = (0x0067) | ((tss & 0xffffff) << 16) |
                   (0x00e9LL << 40) | (((tss >> 24) & 0xff) << 56);
  gdt[SEG_TSS+1] = (tss >> 32);
  set_gdt(gdt, sizeof(SegDesc64) * (NR_SEG + 1));
  set_tr(KSEL(SEG_TSS));
#else
  SegDesc32 *gdt = CPU->gdt;
  TSS32 *tss = &CPU->tss;
  gdt[SEG_KCODE] = SEG32(STA_X | STA_R,   0,     0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG32(STA_W,           0,     0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG32(STA_X | STA_R,   0,     0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG32(STA_W,           0,     0xffffffff, DPL_USER);
  gdt[SEG_TSS]   = SEG16(STS_T32A,      tss, sizeof(*tss)-1, DPL_KERN);
  set_gdt(gdt, sizeof(SegDesc32) * NR_SEG);
  set_tr(KSEL(SEG_TSS));
#endif
}

/*
void __am_thiscpu_setstk0(uintptr_t ss0, uintptr_t esp0) {
  CPU->tss.ss0 = ss0;
  CPU->tss.esp0 = esp0;
}
*/

void __am_thiscpu_halt() {
  while (1) hlt();
}

void __am_othercpu_halt() {
  boot_record()->jmp_code = 0x0000feeb; // (16-bit) jmp .
  for (int cpu = 0; cpu < __am_ncpu; cpu++) {
    if (cpu != _cpu()) {
      __am_lapic_bootap(cpu, 0x7000);
    }
  }
}
