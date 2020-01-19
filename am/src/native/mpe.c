#include <stdlib.h>
#include <stdatomic.h>
#include "platform.h"

#define MAX_SMP 16
static int ncpu = 0;
static int *cpuid = NULL;

int _mpe_init(void (*entry)()) {
  cpuid = __am_private_alloc(sizeof(*cpuid));

  for (int i = 1; i < _ncpu(); i++) {
    if (fork() == 0) {
      __am_init_timer();

      *cpuid = i;
      entry();
    }
  }

  *cpuid = 0;
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
  return (cpuid ? *cpuid : 0);
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  return atomic_exchange((atomic_intptr_t *)addr, newval);
}
