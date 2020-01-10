#include <am.h>
#include <klib.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/prctl.h>
#include <signal.h>
#include "platform.h"

#define MAX_SMP 16
static int ncpu;
static int cpuid;

int _mpe_init(void (*entry)()) {
  char *smp = getenv("smp");
  ncpu = smp ? atoi(smp) : 1;
  assert(0 < ncpu && ncpu <= MAX_SMP);


  int ppid_before_fork = getpid();
  for (int i = 1; i < ncpu; i++) {
    if (fork() == 0) {
      // install a parent death signal in the chlid
      int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
      assert(r != -1);
      assert(getppid() == ppid_before_fork);

      REBASE_ORIGINAL_VAL(cpuid) = i;
      entry();
    }
  }

  REBASE_ORIGINAL_VAL(cpuid) = 0;
  entry();

  printf("MP entry should not return\n");
  assert(0);
  return 0;
}

int _ncpu() {
  return ncpu;
}

int _cpu() {
  return REBASE_ORIGINAL_VAL(cpuid);
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  return atomic_exchange((atomic_intptr_t *)addr, newval);
}
