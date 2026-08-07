#include <xs.h>
#include <cache.h>

int __am_ncpu = 1;  // One core by default
static volatile intptr_t hart_released[MAX_CPU];
static volatile intptr_t mpe_started;
static volatile intptr_t mpe_ready;
static void (*volatile mpe_entry)();

static void init_tls();

void _mpe_setncpu(char arg) {
  if (arg == '\0') {
    __am_ncpu = 1;
  } else {
    assert(arg >= '1' && arg <= '9');
    __am_ncpu = arg - '0';
  }
  assert(0 < __am_ncpu && __am_ncpu <= MAX_CPU);
}

void _mpe_wakeup(int cpu) {
  assert(cpu > 0 && cpu < __am_ncpu);

  /*
   * A secondary hart first needs to be released from reset.  Once it has
   * started, writing its reset-control register again is not a wake-up: a
   * pending CLINT software interrupt is what resumes a hart in WFI.
   */
  if (hart_released[cpu] == 0 && _atomic_xchg(&hart_released[cpu], 1) == 0) {
    uintptr_t reset_addr = HART_CTRL_RESET_REG_BASE +
                           (uintptr_t)cpu * HART_CTRL_RESET_REG_STRIDE;
    asm volatile("fence iorw, iorw" ::: "memory");
    *(volatile uint64_t *)reset_addr = 0;
  }

  /*
   * Four-core XiangShan configurations can start secondary harts before
   * software releases them.  Send MSIP after the reset write so the same
   * path also resumes a hart that has already reached WFI.
   */
  uintptr_t msip_addr = CLINT_MSIP_BASE +
                        (uintptr_t)cpu * CLINT_MSIP_STRIDE;
  asm volatile("fence iorw, iorw" ::: "memory");
  *(volatile uint32_t *)msip_addr = 1;
  asm volatile("fence iorw, iorw" ::: "memory");
}

void _mpe_clear_ipi(int cpu) {
  assert(cpu >= 0 && cpu < __am_ncpu);

  uintptr_t msip_addr = CLINT_MSIP_BASE +
                        (uintptr_t)cpu * CLINT_MSIP_STRIDE;
  *(volatile uint32_t *)msip_addr = 0;
  asm volatile("fence iorw, iorw" ::: "memory");
}

static void init_tls() {
#ifdef DUAL_CORE
  register void* thread_pointer asm("tp");
  extern char _tdata_begin, _tdata_end, _tbss_begin, _tbss_end;
  size_t tdata_size = &_tdata_end - &_tdata_begin;
  size_t tbss_size = &_tbss_end - &_tbss_begin;
  if (tdata_size != 0) {
    memcpy(thread_pointer, &_tdata_begin, tdata_size);
  }
  if (tbss_size != 0) {
    memset(thread_pointer + tdata_size, 0, tbss_size);
  }
#endif
}

int _mpe_start(void (*entry)()) {
  assert(entry != NULL);
  assert(_cpu() == 0);

  if (_atomic_xchg(&mpe_started, 1) == 0) {
    init_tls();
    mpe_entry = entry;
    asm volatile("fence iorw, iorw" ::: "memory");
    _atomic_xchg(&mpe_ready, 1);

    for (int cpu = 1; cpu < __am_ncpu; cpu++) {
      _mpe_wakeup(cpu);
    }
  } else {
    assert(mpe_entry == entry);
  }

  return 0;
}

void __am_mpe_secondary_entry(void) {
  int cpu = _cpu();

  /*
   * Some XiangShan configurations fetch from every hart at reset. At this
   * point MIE is clear, so WFI is not a dependable wait primitive: a later
   * MSIP may remain pending without resuming an already-running hart. Poll
   * until CPU 0 publishes the entry point, then leave unselected harts asleep.
   */
  while (mpe_ready == 0) {
    asm volatile("nop");
  }
  if (cpu >= __am_ncpu) {
    while (1) {
      asm volatile("wfi");
    }
  }

  asm volatile("fence r, rw" ::: "memory");

  init_tls();
  void (*entry)() = mpe_entry;
  assert(entry != NULL);
  entry();

  while (1) {
    asm volatile("wfi");
  }
}

int _mpe_init(void (*entry)()) {
  _mpe_start(entry);
  entry();
  return 0;
}

int _ncpu() {
  return __am_ncpu;
}

int _cpu() {
#ifdef DUAL_CORE
  /*
   * CTE runs application code in S-mode, where mhartid is not readable.
   * start_dual.S assigns every hart a 128 KiB stack/TLS slot rooted at
   * _stack_top, so tp provides a privilege-independent hart identifier.
   */
  register uintptr_t thread_pointer asm("tp");
  extern char _stack_top;
  return ((uintptr_t)thread_pointer - (uintptr_t)&_stack_top) >> 17;
#else
  intptr_t result;
  asm volatile(
    "csrr %0, mhartid;"
    : "=r"(result)
  );
  return result;
#endif
}

intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval) {
  intptr_t result;
  asm volatile(
    "amoswap.d.aqrl %0, %1, (%2);"
    : "=r"(result) 
    : "r"(newval), "r"(addr)
    : "memory"
  );
  return result;
}

intptr_t _atomic_add(volatile intptr_t *addr, intptr_t adder) {
  intptr_t result;
  asm volatile(
    "amoadd.d.aqrl %0, %1, (%2);"
    : "=r"(result)
    : "r"(adder), "r"(addr)
    : "memory"
  );
  return result;
}

void _barrier() {
  static volatile intptr_t sense = 0;
  static volatile intptr_t count = 0;
  static __thread intptr_t threadsense;

  asm volatile("fence;");

  threadsense = !threadsense;
  if (_atomic_add(&count, 1) == _ncpu()-1) {
    count = 0;
    sense = threadsense;
  }
  else while(sense != threadsense)
    ;

  asm volatile("fence;");
}
