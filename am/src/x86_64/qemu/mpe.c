#include <am.h>
#include "x86_64-qemu.h"

struct cpu_local __am_cpuinfo[MAX_CPU] = {};

static void (* volatile user_entry)();
static volatile intptr_t apboot_done = 0;

static void percpu_entry();
static void ap_entry();
static void jump_to(void (*entry)());

int _mpe_init(void (*entry)()) {
  user_entry = entry;
  jump_to(percpu_entry);
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

static void percpu_entry() {
  if (_cpu() == 0) { // bootstrap cpu, boot all aps
    for (int cpu = 1; cpu < __am_ncpu; cpu++) {
      boot_record()->is_ap = 1;
      boot_record()->entry = percpu_entry;
      __am_lapic_bootap(cpu, 0x7c00);
      while (_atomic_xchg(&apboot_done, 0) != 1) {
        pause();
      }
    }
    user_entry();
  } else { // this is an ap
    jump_to(ap_entry);
  }
}

static void ap_entry() {
  percpu_init();
  _atomic_xchg(&apboot_done, 1);
  user_entry();
}

void percpu_init() {
  __am_percpu_initgdt();
//  __am_percpu_initirq();
  __am_percpu_initlapic();
//  __am_percpu_initpg();

}

static void jump_to(void (*entry)()) {
  void *sp = ((uint8_t *)CPU->stack) + sizeof(CPU->stack);
  asm volatile (
#ifndef __x86_64__
    "mov %0, %%esp; call *%1" : : "r"(sp), "r"(entry)
#else
    "mov %0, %%rsp; call *%1" : : "r"(sp), "r"(entry)
#endif
  );
}


void __am_percpu_initgdt() {
#ifndef __x86_64__
  SegDesc *gdt = CPU->gdt;
  TSS *tss = &CPU->tss;
  gdt[SEG_KCODE] = SEG  (STA_X | STA_R,   0,     0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG  (STA_W,           0,     0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG  (STA_X | STA_R,   0,     0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG  (STA_W,           0,     0xffffffff, DPL_USER);
  gdt[SEG_TSS]   = SEG16(STS_T32A,      tss, sizeof(*tss)-1, DPL_KERN);
  set_gdt(gdt, sizeof(SegDesc) * NR_SEG);
  set_tr(KSEL(SEG_TSS));
#else
  SegDesc64 *gdt = CPU->gdt;
  uint64_t tss = (uint64_t)(&CPU->tss);
  gdt[0]         = 0;
  gdt[SEG_KCODE] = 0x0020980000000000LL;
  gdt[SEG_KDATA] = 0x0000920000000000LL;
  gdt[SEG_UCODE] = 0x0020F80000000000LL;
  gdt[SEG_UDATA] = 0x0000F20000000000LL;
  gdt[SEG_TSS+0] = (0x0067) | ((tss & 0xFFFFFF) << 16) |
                   (0x00E9LL << 40) | (((tss >> 24) & 0xFF) << 56);
  gdt[SEG_TSS+1] = (tss >> 32);
  set_gdt(gdt, sizeof(SegDesc64) * (NR_SEG + 1));
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
  cli();
  while (1) hlt();
}

void __am_othercpu_halt() {
  boot_record()->is_ap = 1;
  boot_record()->entry = __am_thiscpu_halt;
  for (int cpu = 0; cpu < __am_ncpu; cpu++) {
    if (cpu != _cpu()) {
      __am_lapic_bootap(cpu, 0x7c00);
    }
  }
}
