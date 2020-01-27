#include <am.h>
#include "x86_64-qemu.h"

struct cpu_local __am_cpuinfo[MAX_CPU] = {};

static void (* volatile user_entry)();
static volatile intptr_t apboot_done = 0;

static void ap_entry();
static void bootcpu_mpe_entry();

int _mpe_init(void (*entry)()) {
  user_entry = entry;
  stack_switch(stack_top(&CPU->stack), bootcpu_mpe_entry, 0);
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

static void bootcpu_mpe_entry() {
  boot_record()->jmp_code = 0x000bfde9; // (16-bit) jmp (0x7c00)
  for (int cpu = 1; cpu < __am_ncpu; cpu++) {
    boot_record()->is_ap = 1;
    __am_lapic_bootap(cpu, 0x7000);
    while (_atomic_xchg(&apboot_done, 0) != 1) {
      pause();
    }
  }
  user_entry();
}

void othercpu_entry() {
  stack_switch(stack_top(&CPU->stack), ap_entry, 0);
}

static void ap_entry() {
  __am_percpu_init();
  _atomic_xchg(&apboot_done, 1);
  user_entry();
}
