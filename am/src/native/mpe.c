#include <am.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>

static int ncpu;
static __thread int cpuid;
static atomic_int cpu_cnt = 0;

void *thread_wrapper(void *entry) {
  cpuid = atomic_fetch_add(&cpu_cnt, 1);
  ((void (*)())entry)();
  printf("MP entry should not return\n");
  exit(1);
  return NULL;
}

int _mpe_init(void (*entry)()) {
  char *smp = getenv("smp");
  ncpu = smp ? atoi(smp) : 1;
  assert(0 < ncpu && ncpu <= 16);

  pthread_t threads[ncpu];

  for (int i = 0; i < ncpu; i++)
    pthread_create(&threads[i], NULL, thread_wrapper, entry);
  for (int i = 0; i < ncpu; i++)
    pthread_join(threads[i], NULL);
  exit(1);
}

int _ncpu() {
  return ncpu;
}

int _cpu() {
  return cpuid;
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  return atomic_exchange((atomic_intptr_t *)addr, newval);
}
