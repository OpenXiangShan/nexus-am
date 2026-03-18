#include <mt_common.h>

static volatile intptr_t secondary_online = 0;

int read_hartid(void) {
  intptr_t hartid = 0;
  asm volatile("csrr %0, mhartid" : "=r"(hartid));
  return (int)hartid;
}

void fence_rw(void) {
  asm volatile("fence rw, rw" ::: "memory");
}

void cpu_relax(void) {
  asm volatile("" ::: "memory");
}

void busy_delay(int cycles) {
  for (volatile int i = 0; i < cycles; i++) {
    asm volatile("" ::: "memory");
  }
}

void spin_lock(volatile intptr_t *lock_word) {
  while (_atomic_xchg(lock_word, 1) == 1) {
    cpu_relax();
  }
  fence_rw();
}

void spin_unlock(volatile intptr_t *lock_word) {
  fence_rw();
  *lock_word = 0;
  fence_rw();
}

void fail_now(const char *sample, int hartid, const char *reason) {
  atomic_printf("[%s] hart %d FAIL: %s\n", sample, hartid, reason);
  _halt(1);
}

void expect(int cond, const char *sample, int hartid, const char *reason) {
  if (!cond) {
    fail_now(sample, hartid, reason);
  }
}

void finish_case(const char *sample, int hartid) {
  _barrier();
  if (hartid == 0) {
    atomic_printf("[%s] PASS\n", sample);
    _halt(0);
  }
  while (1) {
  }
}

void setup_dual_harts(int hartid) {
  _mpe_setncpu('2');

  if (hartid == 0) {
    _mpe_wakeup(1);
    while (secondary_online == 0) {
      cpu_relax();
    }
    fence_rw();
  } else if (hartid == 1) {
    fence_rw();
    secondary_online = 1;
  } else {
    fail_now("boot", hartid, "only hart 0 and hart 1 are covered");
  }

  _barrier();
}
