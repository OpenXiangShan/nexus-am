#include <xs.h>

int __am_ncpu = 1;  // One core by default

void _mpe_setncpu(char arg) {
  __am_ncpu = arg ? atoi(&arg) : 1;
  assert(0 < __am_ncpu && __am_ncpu <= MAX_CPU);
  return;
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

static void init_tls() {
#ifdef DUAL_CORE
  register void* thread_pointer asm("tp");
  extern char _tdata_begin, _tdata_end, _tbss_end;
  size_t tdata_size = &_tdata_end - &_tdata_begin;
  memcpy(thread_pointer, &_tdata_begin, tdata_size);
  size_t tbss_size = &_tbss_end - &_tdata_end;
  memset(thread_pointer + tdata_size, 0, tbss_size);
#endif
}

int _mpe_init(void (*entry)()) {
  init_tls();
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
  intptr_t result;
  asm volatile(
    "amoadd.d %0, %1, (%2);"
    : "=r"(result)
    : "r"(adder), "r"(addr)
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