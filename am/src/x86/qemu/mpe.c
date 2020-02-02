#include "x86-qemu.h"

struct cpu_local __am_cpuinfo[MAX_CPU] = {};
static void (* volatile user_entry)();
static volatile intptr_t ap_ready = 0;

static void call_user_entry() {
  user_entry();
  panic("MPE entry should not return");
}

int _mpe_init(void (*entry)()) {
  user_entry = entry;
  boot_record()->jmp_code = 0x000bfde9; // (16-bit) jmp (0x7c00)
  for (int cpu = 1; cpu < __am_ncpu; cpu++) {
    boot_record()->is_ap = 1;
    __am_lapic_bootap(cpu, (void *)boot_record());
    while (_atomic_xchg(&ap_ready, 0) != 1) {
      pause();
    }
  }
  call_user_entry();
  return 0;
}

static void othercpu_entry() {
  __am_percpu_init();
  _atomic_xchg(&ap_ready, 1);
  call_user_entry();
}

void __am_othercpu_entry() {
  stack_switch_call(stack_top(&CPU->stack), othercpu_entry, 0);
}

int _ncpu() {
  return __am_ncpu;
}

int _cpu(void) {
  return __am_lapic[8] >> 24;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  return xchg(addr, newval);
}

void __am_stop_the_world() {
  boot_record()->jmp_code = 0x0000feeb; // (16-bit) jmp .
  for (int cpu = 0; cpu < __am_ncpu; cpu++) {
    if (cpu != _cpu()) {
      __am_lapic_bootap(cpu, (void *)boot_record());
    }
  }
}
