#include <stdlib.h>
#include <stdatomic.h>
#include "platform.h"

#define MAX_SMP 16
static int ncpu = 0;

int _mpe_init(void (*entry)()) {
  for (int i = 1; i < _ncpu(); i++) {
    if (fork() == 0) {
      thiscpu->cpuid = i;
      __am_init_irq();
      entry();
    }
  }

  entry();
  printf("MP entry should not return\n");
  assert(0);
  return 0;
}

int _ncpu() {
  if (ncpu == 0) {
    char *smp = getenv("smp");
    ncpu = smp ? atoi(smp) : 1;
    assert(0 < ncpu && ncpu <= MAX_SMP);
  }

  return ncpu;
}

int _cpu() {
  return thiscpu->cpuid;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  return atomic_exchange((atomic_intptr_t *)addr, newval);
}
