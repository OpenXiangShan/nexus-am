#include <stdatomic.h>
#include "platform.h"

int __am_mpe_init = 0;

int _mpe_init(void (*entry)()) {
  __am_mpe_init = 1;
  for (int i = 1; i < _ncpu(); i++) {
    if (fork() == 0) {
      thiscpu->cpuid = i;
      __am_init_timer_irq();
      entry();
    }
  }

  entry();
  printf("MP entry should not return\n");
  assert(0);
  return 0;
}

int _ncpu() {
  extern int __am_ncpu;
  return __am_ncpu;
}

int _cpu() {
  return thiscpu->cpuid;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  return atomic_exchange((atomic_intptr_t *)addr, newval);
}
