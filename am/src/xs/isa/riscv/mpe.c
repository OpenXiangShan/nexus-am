#include <xs.h>

int __am_ncpu = 1;  // One core by default

void _mpe_setncpu(char arg) {
  __am_ncpu = arg ? atoi(&arg) : 1;
  assert(0 < __am_ncpu && __am_ncpu <= MAX_CPU);
}

void _mpe_wakeup(int cpu) {
  assert(cpu == 1);
  uint64_t release_addr = 0x39001008;  // Hardware defined
  uint64_t release_val = 0;
  asm volatile(
    "sd %0, (%1);" : : "r"(release_val), "r"(release_addr)
  );
  return;
}

int _mpe_init(void (*entry)()) {
  // TODO: init TLS
  entry();
  return 0;
}

int _ncpu() {
  return __am_ncpu;
}

int _cpu() {
  intptr_t result;
  asm volatile(
    "csrr %0, mhartid;"
    : "=r"(result)
  );
  return result;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile(
    "amoswap.d %0, %1, (%2);"
    : "=r"(result) 
    : "r"(newval), "r"(addr)
  );
  return result;
}

intptr_t _atomic_add(volatile intptr_t *addr, intptr_t adder) {
  return 0;
}