#include <am-x86.h>

int ncpu = 0;
static void (* volatile _entry)();
static intptr_t apboot_done = 0;

static void mp_entry();

int _mpe_init(void (*entry)()) {
  _entry = entry;

  for (int cpu = 1; cpu < ncpu; cpu ++) {
    *(uint16_t*)(0x7c00 + 510) = 0x55aa;
    *(uint32_t*)(0x7000) = 0x007c00ea;  // code for ljmp
    *(uint32_t*)(0x7004) = 0x00000000;
    *(void**)(0x7010) = (void*)mp_entry;
    *(uint32_t*)(0x7020) += 1024; // 1KB bootstrap stack
    lapic_bootap(cpu, 0x7000);
    while (_atomic_xchg(&apboot_done, 0) != 1);
  }

  mp_entry();
  return 0; // never reaches here
}

int _cpu(void) {
  return lapic[8] >> 24;
}

int _ncpu() {
  return ncpu;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile("lock; xchgl %0, %1":
    "+m"(*addr), "=a"(result): "1"(newval): "cc");
  return result;
}

static void mp_entry() {
  if (_cpu() != 0) {
    cpu_initgdt();
    lapic_init();
    ioapic_enable(IRQ_KBD, _cpu());
    cpu_initpte();
  }
  _atomic_xchg(&apboot_done, 1);
  _entry();
}
