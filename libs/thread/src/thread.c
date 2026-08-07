#include "thread.h"

extern int _mpe_start(void (*entry)());
extern void _mpe_wakeup(int cpu);
extern void _mpe_clear_ipi(int cpu);
extern intptr_t _atomic_add(volatile intptr_t *addr, intptr_t adder);
extern void secall_handler_reg(_Context *(*handler)(_Event, _Context *));
extern void stip_handler_reg(_Context *(*handler)(_Event, _Context *));
extern void set_timer_inc(uintptr_t inc);
extern uint32_t uptime(void);
#if defined(__ARCH_RISCV64_XS_DUAL)
extern int g_config_disable_timer;
extern int g_config_disable_external_interrupt;
#endif

#define MAX_TSD_KEYS 128
#define THREAD_ALIGNMENT 16
#define THREAD_OWNER_NONE ((thread_t)-1)
#define THREAD_DESTRUCTOR_ITERATIONS 4
#define THREAD_MAX_HARTS 8
#define THREAD_IDLE_BACKOFF 64
/* The XiangShan CLINT timebase advances once per microsecond. */
#define THREAD_TIMER_TICK 200UL

enum {
  THREAD_READY,
  THREAD_RUNNING,
  THREAD_FINISHED,
};

typedef struct thread_control_block {
  thread_t id;
  void *(*start_routine)(void *);
  void *arg;
  void *retval;
  void *stack_base;
  size_t stack_size;
  _Context *context;
  volatile intptr_t state;
  int detached;
  int joined;
  void *specific[MAX_TSD_KEYS];
  struct thread_control_block *next_all;
  struct thread_control_block *next_run;
} tcb_t;

static volatile intptr_t scheduler_lock;
static volatile intptr_t scheduler_started;
static volatile intptr_t secondary_ready[THREAD_MAX_HARTS];
static volatile intptr_t idle_harts;
static volatile intptr_t wake_pending[THREAD_MAX_HARTS];
static volatile intptr_t scheduler_critical[THREAD_MAX_HARTS];
static volatile intptr_t scheduler_ready;
static int next_idle_hart = 1;
static thread_t next_thread_id = 1;
static tcb_t *thread_list;
static tcb_t *volatile runq_head;
static tcb_t *runq_tail;
static tcb_t main_thread;
static int main_thread_ready;
static tcb_t *current_threads[THREAD_MAX_HARTS];

static thread_key_t tsd_keys[MAX_TSD_KEYS];
static void (*tsd_destructors[MAX_TSD_KEYS])(void *);
static volatile intptr_t tsd_lock;

static _Context *thread_yield_handler(_Event ev, _Context *ctx);
static _Context *thread_timer_handler(_Event ev, _Context *ctx);
static void thread_secondary_entry(void);

static tcb_t *get_current_thread(void) {
  int cpu = _cpu();
  return cpu >= 0 && cpu < THREAD_MAX_HARTS ? current_threads[cpu] : NULL;
}

static void set_current_thread(tcb_t *thread) {
  int cpu = _cpu();
  if (cpu >= 0 && cpu < THREAD_MAX_HARTS) {
    current_threads[cpu] = thread;
  }
}

static void scheduler_relax(void);

static void spin_lock(volatile intptr_t *lock) {
  for (;;) {
    while (*lock != 0) {
      for (int i = 0; i < THREAD_IDLE_BACKOFF; i++) {
        asm volatile("nop");
      }
    }
    if (_atomic_xchg(lock, 1) == 0) {
      return;
    }
  }
}

/* Avoid saturating the XiangShan coherence path while an idle hart polls. */
static void scheduler_relax(void) {
  for (int i = 0; i < THREAD_IDLE_BACKOFF; i++) {
    asm volatile("nop");
  }
}

static void spin_unlock(volatile intptr_t *lock) {
  _atomic_xchg(lock, 0);
}

/* Timer preemption must never re-enter the scheduler while it owns its lock. */
static void scheduler_lock_acquire(void) {
  int cpu = _cpu();
  if (cpu >= 0 && cpu < THREAD_MAX_HARTS) {
    scheduler_critical[cpu]++;
    asm volatile("fence rw, rw" ::: "memory");
  }
  spin_lock(&scheduler_lock);
}

static void scheduler_lock_release(void) {
  spin_unlock(&scheduler_lock);
  int cpu = _cpu();
  if (cpu >= 0 && cpu < THREAD_MAX_HARTS) {
    asm volatile("fence rw, rw" ::: "memory");
    scheduler_critical[cpu]--;
  }
}

static uintptr_t align_up(uintptr_t value, size_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

static void enqueue_locked(tcb_t *tcb) {
  tcb->next_run = NULL;
  if (runq_tail) {
    runq_tail->next_run = tcb;
  } else {
    runq_head = tcb;
  }
  runq_tail = tcb;
}

static intptr_t secondary_hart_mask(void) {
  return ((intptr_t)1 << _ncpu()) - 2;
}

static int handoff_pending_locked(void) {
  for (int cpu = 1; cpu < _ncpu(); cpu++) {
    if (wake_pending[cpu] != 0) {
      return 1;
    }
  }
  return 0;
}

static int handoff_pending(void) {
  for (int cpu = 1; cpu < _ncpu(); cpu++) {
    if (wake_pending[cpu] != 0) {
      return 1;
    }
  }
  return 0;
}

/* scheduler_lock protects idle_harts and the wake-up cursor. */
static int take_idle_hart_locked(void) {
  int ncpu = _ncpu();
  for (int i = 0; i < ncpu - 1; i++) {
    int cpu = next_idle_hart;
    next_idle_hart++;
    if (next_idle_hart >= ncpu) {
      next_idle_hart = 1;
    }
    if (idle_harts & ((intptr_t)1 << cpu)) {
      idle_harts &= ~((intptr_t)1 << cpu);
      wake_pending[cpu] = 1;
      return cpu;
    }
  }
  return -1;
}

static tcb_t *dequeue_locked(void) {
  tcb_t *tcb = runq_head;
  if (!tcb) {
    return NULL;
  }

  runq_head = tcb->next_run;
  if (runq_tail == tcb) {
    runq_tail = NULL;
  }
  tcb->next_run = NULL;
  return tcb;
}

static tcb_t *find_thread_locked(thread_t id) {
  for (tcb_t *tcb = thread_list; tcb; tcb = tcb->next_all) {
    if (tcb->id == id) {
      return tcb;
    }
  }
  return NULL;
}

static void remove_thread_locked(tcb_t *target) {
  tcb_t **link = &thread_list;
  while (*link) {
    if (*link == target) {
      *link = target->next_all;
      target->next_all = NULL;
      return;
    }
    link = &(*link)->next_all;
  }
}

static tcb_t *allocate_tcb_and_stack(size_t stack_size) {
  scheduler_lock_acquire();

  uintptr_t heap_start = (uintptr_t)_heap.start;
  uintptr_t heap_end = (uintptr_t)_heap.end;
  uintptr_t tcb_addr = align_up(heap_start, THREAD_ALIGNMENT);
  uintptr_t stack_addr = align_up(tcb_addr + sizeof(tcb_t), THREAD_ALIGNMENT);

  if (tcb_addr > heap_end || stack_addr > heap_end ||
      stack_size > heap_end - stack_addr) {
    scheduler_lock_release();
    return NULL;
  }

  _heap.start = (void *)(stack_addr + stack_size);
  scheduler_lock_release();
  return (tcb_t *)tcb_addr;
}

static void install_scheduler_on_cpu(void) {
#if defined(__ARCH_RISCV64_XS_DUAL)
  /* CPU 0 publishes these CTE-wide settings before it releases secondaries. */
  if (_cpu() == 0) {
    g_config_disable_timer = 0;
    g_config_disable_external_interrupt = 1;
  }
  /* init_timer() consumes this while this hart is still in M-mode. */
  set_timer_inc(THREAD_TIMER_TICK);
#endif
  _cte_init(NULL);
  secall_handler_reg(thread_yield_handler);
#if defined(__ARCH_RISCV64_XS_DUAL)
  stip_handler_reg(thread_timer_handler);
  asm volatile("csrs sie, %0" : : "r"((1 << 5) | (1 << 1)));
  _intr_write(1);
#endif
}

static void ensure_scheduler(void) {
  if (_atomic_xchg(&scheduler_started, 1) == 0) {
    install_scheduler_on_cpu();
    _mpe_start(thread_secondary_entry);

    /* Start with every secondary parked in the scheduler before queuing work. */
    for (int cpu = 1; cpu < _ncpu(); cpu++) {
      while (secondary_ready[cpu] == 0) {
        asm volatile("fence r, rw" ::: "memory");
        scheduler_relax();
      }
    }
    idle_harts = secondary_hart_mask();
    asm volatile("fence rw, rw" ::: "memory");
    _atomic_xchg(&scheduler_ready, 1);
  }
}

static void wake_idle_hart(int cpu) {
  if (cpu > 0) {
    _mpe_wakeup(cpu);
  }
}

static tcb_t *main_thread_locked(_Context *ctx) {
  tcb_t *current = get_current_thread();
  if (current || _cpu() != 0) {
    return current;
  }

  if (!main_thread_ready) {
    main_thread.id = 0;
    main_thread.context = ctx;
    main_thread.state = THREAD_RUNNING;
    main_thread_ready = 1;
  }
  set_current_thread(&main_thread);
  return &main_thread;
}

/* A FIFO thread may resume on another hart, so preserve that hart's tp. */
static _Context *activate_locked(tcb_t *thread, _Context *hart_context) {
  thread->state = THREAD_RUNNING;
  thread->context->gpr[4] = hart_context->gpr[4];  // x4 == tp
  set_current_thread(thread);
  return thread->context;
}

static _Context *wait_for_work(int cpu, _Context *hart_context) {
  intptr_t cpu_bit = (intptr_t)1 << cpu;
  for (;;) {
    _mpe_clear_ipi(cpu);

    scheduler_lock_acquire();
    idle_harts &= ~cpu_bit;
    int selected = cpu > 0 && wake_pending[cpu] != 0;
    int waiting_for_other = handoff_pending_locked() && !selected;
    tcb_t *next = (!waiting_for_other) ?
                      dequeue_locked() : NULL;
    if (next) {
      if (selected) {
        wake_pending[cpu] = 0;
      }
      _Context *next_context = activate_locked(next, hart_context);
      scheduler_lock_release();
      return next_context;
    }
    scheduler_lock_release();

    /* S-mode WFI does not resume reliably from CLINT MSIP in this emu. */
    while (waiting_for_other ? handoff_pending() : runq_head == NULL) {
      scheduler_relax();
    }
  }
}

static _Context *schedule_current(_Context *ctx) {
  int cpu = _cpu();
  scheduler_lock_acquire();

  tcb_t *current = main_thread_locked(ctx);
  if (current && current->state == THREAD_RUNNING) {
    current->context = ctx;
    current->state = THREAD_READY;
    enqueue_locked(current);
  }

  tcb_t *next = handoff_pending_locked() ? NULL : dequeue_locked();
  if (next) {
    _Context *next_context = activate_locked(next, ctx);
    scheduler_lock_release();
    return next_context;
  }

  set_current_thread(NULL);
  scheduler_lock_release();
  return wait_for_work(cpu, ctx);
}

static _Context *thread_yield_handler(_Event ev, _Context *ctx) {
  return ev.event == _EVENT_YIELD ? schedule_current(ctx) : ctx;
}

static _Context *thread_timer_handler(_Event ev, _Context *ctx) {
  if (ev.event != _EVENT_IRQ_TIMER) {
    return ctx;
  }

  int cpu = _cpu();
  if (cpu < 0 || cpu >= THREAD_MAX_HARTS || !scheduler_ready ||
      scheduler_critical[cpu] != 0 || get_current_thread() == NULL) {
    return ctx;
  }

  return schedule_current(ctx);
}

static void thread_secondary_entry(void) {
  int cpu = _cpu();
  _mpe_clear_ipi(cpu);

  /*
   * Do not park in WFI here.  On the current XiangShan model a secondary can
   * remain asleep after its MSIP is asserted, so the scheduler's idle path
   * polls the shared ready queue instead.  The MPE release remains responsible
   * only for bringing the hart out of reset.
   */
  install_scheduler_on_cpu();
  _atomic_xchg(&secondary_ready[cpu], 1);
  asm volatile("fence rw, rw" ::: "memory");
  _yield();

  while (1) {
    asm volatile("nop");
  }
}

static void run_tsd_destructors(tcb_t *tcb) {
  for (int pass = 0; pass < THREAD_DESTRUCTOR_ITERATIONS; pass++) {
    int called_destructor = 0;

    /*
     * Multiple threads can exit at once. Snapshot every key while holding the
     * TSD lock once, then run callbacks without it. This avoids hundreds of
     * contended AMOs on a many-hart exit path and still permits destructors to
     * install another value for the next POSIX destructor pass.
     */
    void (*destructors[MAX_TSD_KEYS])(void *);
    void *values[MAX_TSD_KEYS];
    spin_lock(&tsd_lock);
    for (int i = 0; i < MAX_TSD_KEYS; i++) {
      destructors[i] = tsd_keys[i] ? tsd_destructors[i] : NULL;
      values[i] = tcb->specific[i];
      tcb->specific[i] = NULL;
    }
    spin_unlock(&tsd_lock);

    for (int i = 0; i < MAX_TSD_KEYS; i++) {
      if (destructors[i] && values[i]) {
        called_destructor = 1;
        destructors[i](values[i]);
      }
    }
    if (!called_destructor) {
      break;
    }
  }
}

static void finish_current_thread(void *retval) {
  tcb_t *tcb = get_current_thread();
  if (!tcb || tcb == &main_thread) {
    return;
  }

  run_tsd_destructors(tcb);

  scheduler_lock_acquire();
  tcb->retval = retval;
  tcb->state = THREAD_FINISHED;
  if (tcb->detached) {
    remove_thread_locked(tcb);
  }
  scheduler_lock_release();
}

static void thread_entry_wrapper(void *tcb_ptr) {
  tcb_t *tcb = (tcb_t *)tcb_ptr;
  set_current_thread(tcb);
  thread_exit(tcb->start_routine(tcb->arg));
}

int thread_create(thread_t *thread, const thread_attr_t *attr,
                  void *(*start_routine)(void *), void *arg) {
  if (!thread || !start_routine) {
    return THREAD_INVAL;
  }

  size_t stack_size = THREAD_STACK_DEFAULT;
  int detach_state = THREAD_CREATE_JOINABLE;
  if (attr) {
    if (attr->stack_size < THREAD_STACK_MIN ||
        (attr->detach_state != THREAD_CREATE_JOINABLE &&
         attr->detach_state != THREAD_CREATE_DETACHED)) {
      return THREAD_INVAL;
    }
    stack_size = attr->stack_size;
    detach_state = attr->detach_state;
  }

  ensure_scheduler();

  tcb_t *tcb = allocate_tcb_and_stack(stack_size);
  if (!tcb) {
    return THREAD_NOMEM;
  }

  for (int i = 0; i < MAX_TSD_KEYS; i++) {
    tcb->specific[i] = NULL;
  }
  tcb->start_routine = start_routine;
  tcb->arg = arg;
  tcb->retval = NULL;
  tcb->stack_base = (void *)align_up((uintptr_t)tcb + sizeof(tcb_t),
                                     THREAD_ALIGNMENT);
  tcb->stack_size = stack_size;
  tcb->state = THREAD_READY;
  tcb->detached = detach_state == THREAD_CREATE_DETACHED;
  tcb->joined = 0;
  tcb->next_all = NULL;
  tcb->next_run = NULL;

  _Area stack = {
    .start = tcb->stack_base,
    .end = (char *)tcb->stack_base + stack_size,
  };
  tcb->context = _kcontext(stack, thread_entry_wrapper, tcb);
  if (!tcb->context) {
    return THREAD_ERROR;
  }
  scheduler_lock_acquire();
  tcb->id = next_thread_id++;
  tcb->next_all = thread_list;
  thread_list = tcb;
  enqueue_locked(tcb);
  int wake_cpu = take_idle_hart_locked();
  scheduler_lock_release();

  *thread = tcb->id;
  wake_idle_hart(wake_cpu);
  return THREAD_SUCCESS;
}

int thread_join(thread_t thread, void **retval) {
  if (thread == 0 || thread == thread_self()) {
    return THREAD_INVAL;
  }

  for (;;) {
    scheduler_lock_acquire();
    tcb_t *tcb = find_thread_locked(thread);
    if (!tcb || tcb->detached || tcb->joined) {
      scheduler_lock_release();
      return THREAD_INVAL;
    }

    if (tcb->state == THREAD_FINISHED) {
      if (retval) {
        *retval = tcb->retval;
      }
      tcb->joined = 1;
      remove_thread_locked(tcb);
      scheduler_lock_release();
      return THREAD_SUCCESS;
    }
    scheduler_lock_release();
    thread_yield();
  }
}

int thread_detach(thread_t thread) {
  if (thread == 0) {
    return THREAD_INVAL;
  }

  scheduler_lock_acquire();
  tcb_t *tcb = find_thread_locked(thread);
  if (!tcb || tcb->detached || tcb->joined) {
    scheduler_lock_release();
    return THREAD_INVAL;
  }

  tcb->detached = 1;
  if (tcb->state == THREAD_FINISHED) {
    remove_thread_locked(tcb);
  }
  scheduler_lock_release();
  return THREAD_SUCCESS;
}

void thread_exit(void *retval) {
  tcb_t *current = get_current_thread();
  if (!current || current == &main_thread) {
    return;
  }

  finish_current_thread(retval);
  thread_yield();

  while (1) {
    asm volatile("wfi");
  }
}

thread_t thread_self(void) {
  tcb_t *current = get_current_thread();
  return current ? current->id : 0;
}

int thread_equal(thread_t t1, thread_t t2) {
  return t1 == t2;
}

int thread_yield(void) {
  ensure_scheduler();
  _yield();
  return THREAD_SUCCESS;
}

int thread_attr_init(thread_attr_t *attr) {
  if (!attr) {
    return THREAD_INVAL;
  }
  attr->stack_size = THREAD_STACK_DEFAULT;
  attr->priority = 0;
  attr->detach_state = THREAD_CREATE_JOINABLE;
  return THREAD_SUCCESS;
}

int thread_attr_destroy(thread_attr_t *attr) {
  return attr ? THREAD_SUCCESS : THREAD_INVAL;
}

int thread_attr_setstacksize(thread_attr_t *attr, size_t stacksize) {
  if (!attr || stacksize < THREAD_STACK_MIN) {
    return THREAD_INVAL;
  }
  attr->stack_size = stacksize;
  return THREAD_SUCCESS;
}

int thread_attr_getstacksize(const thread_attr_t *attr, size_t *stacksize) {
  if (!attr || !stacksize) {
    return THREAD_INVAL;
  }
  *stacksize = attr->stack_size;
  return THREAD_SUCCESS;
}

int thread_attr_setdetachstate(thread_attr_t *attr, int detachstate) {
  if (!attr || (detachstate != THREAD_CREATE_JOINABLE &&
                detachstate != THREAD_CREATE_DETACHED)) {
    return THREAD_INVAL;
  }
  attr->detach_state = detachstate;
  return THREAD_SUCCESS;
}

int thread_attr_getdetachstate(const thread_attr_t *attr, int *detachstate) {
  if (!attr || !detachstate) {
    return THREAD_INVAL;
  }
  *detachstate = attr->detach_state;
  return THREAD_SUCCESS;
}

int thread_mutex_init(thread_mutex_t *mutex, const thread_mutexattr_t *attr) {
  if (!mutex) {
    return THREAD_INVAL;
  }
  mutex->lock = 0;
  mutex->owner = THREAD_OWNER_NONE;
  mutex->type = attr ? attr->type : THREAD_MUTEX_NORMAL;
  if (mutex->type != THREAD_MUTEX_NORMAL &&
      mutex->type != THREAD_MUTEX_RECURSIVE &&
      mutex->type != THREAD_MUTEX_ERRORCHECK) {
    return THREAD_INVAL;
  }
  return THREAD_SUCCESS;
}

int thread_mutex_destroy(thread_mutex_t *mutex) {
  if (!mutex) {
    return THREAD_INVAL;
  }
  return mutex->lock == 0 ? THREAD_SUCCESS : THREAD_BUSY;
}

int thread_mutex_lock(thread_mutex_t *mutex) {
  if (!mutex) {
    return THREAD_INVAL;
  }

  thread_t self = thread_self();
  for (;;) {
    if (mutex->lock != 0 && mutex->owner == self) {
      if (mutex->type == THREAD_MUTEX_RECURSIVE) {
        _atomic_add(&mutex->lock, 1);
        return THREAD_SUCCESS;
      }
      if (mutex->type == THREAD_MUTEX_ERRORCHECK) {
        return THREAD_BUSY;
      }
    }

    if (mutex->lock == 0 && _atomic_xchg(&mutex->lock, 1) == 0) {
      mutex->owner = self;
      return THREAD_SUCCESS;
    }
    thread_yield();
  }
}

int thread_mutex_trylock(thread_mutex_t *mutex) {
  if (!mutex) {
    return THREAD_INVAL;
  }

  thread_t self = thread_self();
  if (mutex->lock != 0 && mutex->owner == self) {
    if (mutex->type == THREAD_MUTEX_RECURSIVE) {
      _atomic_add(&mutex->lock, 1);
      return THREAD_SUCCESS;
    }
    return mutex->type == THREAD_MUTEX_ERRORCHECK ? THREAD_BUSY : THREAD_BUSY;
  }

  if (_atomic_xchg(&mutex->lock, 1) != 0) {
    return THREAD_BUSY;
  }
  mutex->owner = self;
  return THREAD_SUCCESS;
}

int thread_mutex_unlock(thread_mutex_t *mutex) {
  if (!mutex || mutex->lock == 0 || mutex->owner != thread_self()) {
    return THREAD_PERM;
  }

  if (mutex->type == THREAD_MUTEX_RECURSIVE && mutex->lock > 1) {
    _atomic_add(&mutex->lock, -1);
  } else {
    mutex->owner = THREAD_OWNER_NONE;
    _atomic_xchg(&mutex->lock, 0);
  }
  return THREAD_SUCCESS;
}

int thread_mutexattr_init(thread_mutexattr_t *attr) {
  if (!attr) {
    return THREAD_INVAL;
  }
  attr->type = THREAD_MUTEX_NORMAL;
  return THREAD_SUCCESS;
}

int thread_mutexattr_destroy(thread_mutexattr_t *attr) {
  return attr ? THREAD_SUCCESS : THREAD_INVAL;
}

int thread_mutexattr_settype(thread_mutexattr_t *attr, int type) {
  if (!attr || (type != THREAD_MUTEX_NORMAL &&
                type != THREAD_MUTEX_RECURSIVE &&
                type != THREAD_MUTEX_ERRORCHECK)) {
    return THREAD_INVAL;
  }
  attr->type = type;
  return THREAD_SUCCESS;
}

int thread_mutexattr_gettype(const thread_mutexattr_t *attr, int *type) {
  if (!attr || !type) {
    return THREAD_INVAL;
  }
  *type = attr->type;
  return THREAD_SUCCESS;
}

int thread_cond_init(thread_cond_t *cond, const thread_condattr_t *attr) {
  if (!cond) {
    return THREAD_INVAL;
  }
  (void)attr;
  cond->lock = 0;
  cond->wait_count = 0;
  cond->wake_count = 0;
  cond->mutex = NULL;
  return THREAD_SUCCESS;
}

int thread_cond_destroy(thread_cond_t *cond) {
  if (!cond) {
    return THREAD_INVAL;
  }
  spin_lock(&cond->lock);
  int result = cond->wait_count == 0 ? THREAD_SUCCESS : THREAD_BUSY;
  spin_unlock(&cond->lock);
  return result;
}

int thread_cond_wait(thread_cond_t *cond, thread_mutex_t *mutex) {
  if (!cond || !mutex) {
    return THREAD_INVAL;
  }

  spin_lock(&cond->lock);
  if (cond->wait_count != 0 && cond->mutex != mutex) {
    spin_unlock(&cond->lock);
    return THREAD_INVAL;
  }
  cond->wait_count++;
  cond->mutex = mutex;
  spin_unlock(&cond->lock);

  int result = thread_mutex_unlock(mutex);
  if (result != THREAD_SUCCESS) {
    spin_lock(&cond->lock);
    cond->wait_count--;
    if (cond->wait_count == 0) {
      cond->mutex = NULL;
    }
    spin_unlock(&cond->lock);
    return result;
  }

  for (;;) {
    spin_lock(&cond->lock);
    if (cond->wake_count > 0) {
      cond->wake_count--;
      cond->wait_count--;
      if (cond->wait_count == 0) {
        cond->mutex = NULL;
      }
      spin_unlock(&cond->lock);
      break;
    }
    spin_unlock(&cond->lock);
    thread_yield();
  }

  return thread_mutex_lock(mutex);
}

int thread_cond_timedwait(thread_cond_t *cond, thread_mutex_t *mutex,
                          const thread_time_t *abstime) {
  if (!cond || !mutex || !abstime) {
    return THREAD_INVAL;
  }

  spin_lock(&cond->lock);
  if (cond->wait_count != 0 && cond->mutex != mutex) {
    spin_unlock(&cond->lock);
    return THREAD_INVAL;
  }
  cond->wait_count++;
  cond->mutex = mutex;
  spin_unlock(&cond->lock);

  int result = thread_mutex_unlock(mutex);
  if (result != THREAD_SUCCESS) {
    spin_lock(&cond->lock);
    cond->wait_count--;
    if (cond->wait_count == 0) {
      cond->mutex = NULL;
    }
    spin_unlock(&cond->lock);
    return result;
  }

  int timed_out = 0;
  for (;;) {
    spin_lock(&cond->lock);
    if (cond->wake_count > 0) {
      cond->wake_count--;
      cond->wait_count--;
      if (cond->wait_count == 0) {
        cond->mutex = NULL;
      }
      spin_unlock(&cond->lock);
      break;
    }
    if ((int32_t)(uptime() - *abstime) >= 0) {
      cond->wait_count--;
      if (cond->wait_count == 0) {
        cond->mutex = NULL;
      }
      timed_out = 1;
      spin_unlock(&cond->lock);
      break;
    }
    spin_unlock(&cond->lock);
    thread_yield();
  }

  result = thread_mutex_lock(mutex);
  return result == THREAD_SUCCESS && timed_out ? THREAD_TIMEOUT : result;
}

int thread_cond_signal(thread_cond_t *cond) {
  if (!cond) {
    return THREAD_INVAL;
  }
  spin_lock(&cond->lock);
  if (cond->wake_count < cond->wait_count) {
    cond->wake_count++;
  }
  spin_unlock(&cond->lock);
  return THREAD_SUCCESS;
}

int thread_cond_broadcast(thread_cond_t *cond) {
  if (!cond) {
    return THREAD_INVAL;
  }
  spin_lock(&cond->lock);
  cond->wake_count = cond->wait_count;
  spin_unlock(&cond->lock);
  return THREAD_SUCCESS;
}

int thread_once(thread_once_t *once_control, void (*init_routine)(void)) {
  if (!once_control || !init_routine) {
    return THREAD_INVAL;
  }
  if (_atomic_add(&once_control->done, 0) != 0) {
    return THREAD_SUCCESS;
  }

  if (_atomic_xchg(&once_control->in_progress, 1) == 0) {
    init_routine();
    _atomic_xchg(&once_control->done, 1);
    _atomic_xchg(&once_control->in_progress, 0);
    return THREAD_SUCCESS;
  }

  while (_atomic_add(&once_control->done, 0) == 0) {
    thread_yield();
  }
  return THREAD_SUCCESS;
}

int thread_key_create(thread_key_t *key, void (*destructor)(void *)) {
  if (!key) {
    return THREAD_INVAL;
  }

  spin_lock(&tsd_lock);
  for (int i = 0; i < MAX_TSD_KEYS; i++) {
    if (tsd_keys[i] == 0) {
      tsd_keys[i] = 1;
      tsd_destructors[i] = destructor;
      *key = i + 1;
      spin_unlock(&tsd_lock);
      return THREAD_SUCCESS;
    }
  }
  spin_unlock(&tsd_lock);
  return THREAD_NOMEM;
}

int thread_key_delete(thread_key_t key) {
  if (key == 0 || key > MAX_TSD_KEYS) {
    return THREAD_INVAL;
  }

  scheduler_lock_acquire();
  spin_lock(&tsd_lock);
  if (tsd_keys[key - 1] == 0) {
    spin_unlock(&tsd_lock);
    scheduler_lock_release();
    return THREAD_INVAL;
  }
  tsd_keys[key - 1] = 0;
  tsd_destructors[key - 1] = NULL;
  for (tcb_t *tcb = thread_list; tcb; tcb = tcb->next_all) {
    tcb->specific[key - 1] = NULL;
  }
  main_thread.specific[key - 1] = NULL;
  spin_unlock(&tsd_lock);
  scheduler_lock_release();
  return THREAD_SUCCESS;
}

void *thread_getspecific(thread_key_t key) {
  tcb_t *current = get_current_thread();
  if (key == 0 || key > MAX_TSD_KEYS || !current) {
    return NULL;
  }

  spin_lock(&tsd_lock);
  int active = tsd_keys[key - 1] != 0;
  spin_unlock(&tsd_lock);
  return active ? current->specific[key - 1] : NULL;
}

int thread_setspecific(thread_key_t key, const void *value) {
  tcb_t *current = get_current_thread();
  if (key == 0 || key > MAX_TSD_KEYS || !current) {
    return THREAD_INVAL;
  }

  spin_lock(&tsd_lock);
  int active = tsd_keys[key - 1] != 0;
  if (active) {
    current->specific[key - 1] = (void *)value;
  }
  spin_unlock(&tsd_lock);
  return active ? THREAD_SUCCESS : THREAD_INVAL;
}

int thread_barrier_init(thread_barrier_t *barrier, unsigned count) {
  if (!barrier || count == 0) {
    return THREAD_INVAL;
  }
  barrier->lock = 0;
  barrier->count = 0;
  barrier->generation = 0;
  barrier->required = count;
  return THREAD_SUCCESS;
}

int thread_barrier_destroy(thread_barrier_t *barrier) {
  if (!barrier) {
    return THREAD_INVAL;
  }
  spin_lock(&barrier->lock);
  int result = barrier->count == 0 ? THREAD_SUCCESS : THREAD_BUSY;
  spin_unlock(&barrier->lock);
  return result;
}

int thread_barrier_wait(thread_barrier_t *barrier) {
  if (!barrier) {
    return THREAD_INVAL;
  }

  spin_lock(&barrier->lock);
  intptr_t generation = barrier->generation;
  barrier->count++;
  if ((unsigned)barrier->count == barrier->required) {
    barrier->count = 0;
    barrier->generation++;
    spin_unlock(&barrier->lock);
    return THREAD_SUCCESS;
  }
  spin_unlock(&barrier->lock);

  while (barrier->generation == generation) {
    thread_yield();
  }
  return THREAD_SUCCESS;
}
