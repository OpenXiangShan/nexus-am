#include <am.h>
#include <klib.h>
#include <thread.h>
#include <xsextra.h>
#include <xs.h>

#ifndef THREAD_SMOKE_NCPU
#define THREAD_SMOKE_NCPU 1
#endif

#if THREAD_SMOKE_NCPU < 1 || THREAD_SMOKE_NCPU > MAX_CPU
#error "THREAD_SMOKE_NCPU must be in [1, MAX_CPU]"
#endif

#define HART_COVERAGE_TIMEOUT_US 50000U
#define HART_COVERAGE_BACKOFF 64
#define PREEMPT_BACKOFF 64
#define PREEMPT_SPIN_LIMIT 256
#define PREEMPT_WORKER_COUNT (THREAD_SMOKE_NCPU + 1)

#ifdef THREAD_SMOKE_TRACE
#define SMOKE_TRACE(stage) atomic_printf(stage)
#else
#define SMOKE_TRACE(stage) ((void)0)
#endif

static thread_barrier_t workers_ready;
static thread_mutex_t counter_lock;
static thread_mutex_t condition_lock;
static thread_cond_t condition;
static thread_once_t once_control;
static thread_key_t worker_key;
static volatile intptr_t counter;
static volatile intptr_t destructor_count;
static volatile intptr_t once_count;
static volatile intptr_t condition_ready;
static volatile intptr_t hart_coverage_lock;
static volatile intptr_t worker_hart_mask;
static volatile intptr_t worker_hart_count;
static volatile intptr_t hart_coverage_timed_out;
static volatile intptr_t preempt_started;
static volatile intptr_t preempt_release;
static volatile intptr_t preempt_timed_out;

static void lock_hart_coverage(void) {
  while (_atomic_xchg(&hart_coverage_lock, 1) != 0) {
    asm volatile("nop");
  }
}

static void unlock_hart_coverage(void) {
  _atomic_xchg(&hart_coverage_lock, 0);
}

/*
 * Do not yield before every worker has arrived. A hart running one of these
 * workers remains occupied, so reaching THREAD_SMOKE_NCPU proves that the
 * scheduler dispatched workers to every configured hart.
 */
static int wait_for_hart_coverage(void) {
  uint32_t deadline = uptime() + HART_COVERAGE_TIMEOUT_US;
  while (worker_hart_count != THREAD_SMOKE_NCPU) {
    if ((int32_t)(uptime() - deadline) >= 0) {
      _atomic_xchg(&hart_coverage_timed_out, 1);
      return THREAD_TIMEOUT;
    }
    for (int i = 0; i < HART_COVERAGE_BACKOFF; i++) {
      asm volatile("nop");
    }
  }
  return THREAD_SUCCESS;
}

static void key_destructor(void *value) {
  if (value) {
    _atomic_add(&destructor_count, 1);
  }
}

static void once_initializer(void) {
  _atomic_add(&once_count, 1);
}

static void *worker(void *arg) {
  int cpu = _cpu();
  if (thread_self() == 0 || cpu < 0 || cpu >= _ncpu()) {
    return (void *)0;
  }

  lock_hart_coverage();
  worker_hart_mask |= (intptr_t)1 << cpu;
  _atomic_add(&worker_hart_count, 1);
  unlock_hart_coverage();
  if (wait_for_hart_coverage() != THREAD_SUCCESS) {
    return NULL;
  }

  if (thread_once(&once_control, once_initializer) != THREAD_SUCCESS ||
      thread_setspecific(worker_key, arg) != THREAD_SUCCESS ||
      thread_getspecific(worker_key) != arg) {
    return (void *)0;
  }
  if (thread_barrier_wait(&workers_ready) != THREAD_SUCCESS) {
    return (void *)0;
  }
  if (thread_mutex_lock(&counter_lock) != THREAD_SUCCESS) {
    return (void *)0;
  }
  counter++;
  thread_mutex_unlock(&counter_lock);
  return arg;
}

/*
 * These workers never call thread_yield(). With more workers than harts, the
 * final worker can start before the spin bound only if a timer interrupt switches
 * away from one of the busy workers.
 */
static void *preempt_worker(void *arg) {
  _atomic_add(&preempt_started, 1);

  for (unsigned int spins = 0; preempt_release == 0; spins++) {
    if (_atomic_add(&preempt_started, 0) == PREEMPT_WORKER_COUNT) {
      _atomic_xchg(&preempt_release, 1);
      break;
    }
    if (spins == PREEMPT_SPIN_LIMIT) {
      _atomic_xchg(&preempt_timed_out, 1);
      _atomic_xchg(&preempt_release, 1);
      break;
    }
    for (int i = 0; i < PREEMPT_BACKOFF; i++) {
      asm volatile("nop");
    }
  }

  return arg;
}

static void *condition_waiter(void *arg) {
  if (thread_mutex_lock(&condition_lock) != THREAD_SUCCESS) {
    return NULL;
  }
  while (!condition_ready) {
    if (thread_cond_wait(&condition, &condition_lock) != THREAD_SUCCESS) {
      thread_mutex_unlock(&condition_lock);
      return NULL;
    }
  }
  int unlocked = thread_mutex_unlock(&condition_lock) == THREAD_SUCCESS;
  return unlocked ? arg : NULL;
}

static void *condition_signaler(void *arg) {
  if (thread_mutex_lock(&condition_lock) != THREAD_SUCCESS) {
    return NULL;
  }
  condition_ready = 1;
  int result = thread_cond_signal(&condition);
  thread_mutex_unlock(&condition_lock);
  return result == THREAD_SUCCESS ? arg : NULL;
}

int main(void) {
  atomic_printf("M");
  _mpe_setncpu('0' + THREAD_SMOKE_NCPU);
  if (thread_barrier_init(&workers_ready, THREAD_SMOKE_NCPU) != THREAD_SUCCESS ||
      thread_mutex_init(&counter_lock, NULL) != THREAD_SUCCESS ||
      thread_mutex_init(&condition_lock, NULL) != THREAD_SUCCESS ||
      thread_cond_init(&condition, NULL) != THREAD_SUCCESS ||
      thread_key_create(&worker_key, key_destructor) != THREAD_SUCCESS) {
    return 1;
  }
  thread_t workers[THREAD_SMOKE_NCPU];
  void *worker_results[THREAD_SMOKE_NCPU];
  thread_t waiter;
  thread_t signaler;
  thread_t preempt_workers[PREEMPT_WORKER_COUNT];
  for (int i = 0; i < THREAD_SMOKE_NCPU; i++) {
    worker_results[i] = NULL;
    if (thread_create(&workers[i], NULL, worker, (void *)(intptr_t)(i + 1)) !=
        THREAD_SUCCESS) {
      return 2;
    }
  }
  if (thread_create(&waiter, NULL, condition_waiter,
                    (void *)(intptr_t)(THREAD_SMOKE_NCPU + 1)) != THREAD_SUCCESS ||
      thread_create(&signaler, NULL, condition_signaler,
                    (void *)(intptr_t)(THREAD_SMOKE_NCPU + 2)) != THREAD_SUCCESS) {
    return 2;
  }
  void *waiter_result = NULL;
  void *signaler_result = NULL;
  for (int i = 0; i < THREAD_SMOKE_NCPU; i++) {
    if (thread_join(workers[i], &worker_results[i]) != THREAD_SUCCESS) {
      return 3;
    }
  }
  if (thread_join(waiter, &waiter_result) != THREAD_SUCCESS) {
    return 3;
  }
  if (thread_join(signaler, &signaler_result) != THREAD_SUCCESS) {
    return 3;
  }
  SMOKE_TRACE("C");
  for (int i = 0; i < PREEMPT_WORKER_COUNT; i++) {
    if (thread_create(&preempt_workers[i], NULL, preempt_worker,
                      (void *)(intptr_t)(i + 1)) != THREAD_SUCCESS) {
      return 8;
    }
  }
  SMOKE_TRACE("R");
  for (int i = 0; i < PREEMPT_WORKER_COUNT; i++) {
    void *preempt_result = NULL;
    if (thread_join(preempt_workers[i], &preempt_result) != THREAD_SUCCESS ||
        preempt_result != (void *)(intptr_t)(i + 1)) {
      return 8;
    }
  }
  SMOKE_TRACE("P");
  thread_time_t expired = uptime() - 1;
  if (thread_mutex_lock(&condition_lock) != THREAD_SUCCESS) {
    return 4;
  }
  int timedwait_result = thread_cond_timedwait(&condition, &condition_lock, &expired);
  if (thread_mutex_unlock(&condition_lock) != THREAD_SUCCESS ||
      timedwait_result != THREAD_TIMEOUT) {
    return 5;
  }
  for (int i = 0; i < THREAD_SMOKE_NCPU; i++) {
    if (worker_results[i] != (void *)(intptr_t)(i + 1)) {
      return 6;
    }
  }
  intptr_t expected_hart_mask = ((intptr_t)1 << THREAD_SMOKE_NCPU) - 1;
  if (hart_coverage_timed_out || worker_hart_mask != expected_hart_mask) {
    return 7;
  }
  if (preempt_timed_out || preempt_started != PREEMPT_WORKER_COUNT) {
    return 8;
  }
  if (counter != THREAD_SMOKE_NCPU || once_count != 1 ||
      destructor_count != THREAD_SMOKE_NCPU ||
      waiter_result != (void *)(intptr_t)(THREAD_SMOKE_NCPU + 1) ||
      signaler_result != (void *)(intptr_t)(THREAD_SMOKE_NCPU + 2) ||
      !condition_ready ||
      thread_key_delete(worker_key) != THREAD_SUCCESS ||
      thread_cond_destroy(&condition) != THREAD_SUCCESS ||
      thread_mutex_destroy(&condition_lock) != THREAD_SUCCESS ||
      thread_mutex_destroy(&counter_lock) != THREAD_SUCCESS ||
      thread_barrier_destroy(&workers_ready) != THREAD_SUCCESS) {
    return 6;
  }
  s_atomic_printf("H%02xTS", (unsigned int)worker_hart_mask);
  return 0;
}
