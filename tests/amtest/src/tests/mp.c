#include <amtest.h>

/*
 * To build mp test, try `make ARCH=riscv64-xs-dual mainargs='m2'`
 */

static volatile intptr_t sum = 0;
static volatile intptr_t atomic_sum = 0;
static volatile intptr_t print_lock = 0;

void __am_uartlite_putchar(char ch);

void mp_print() {
  _mpe_wakeup(1);
  while (_atomic_xchg(&print_lock, 1) == 1);
  printf("My CPU ID is %d, nrCPU is %d\n", _cpu(), _ncpu());
  _atomic_xchg(&print_lock, 0);
  for (int i = 0; i < 100; i++) {
    sum++;
    _atomic_add(&atomic_sum, 1);
  }
}

void finalize() {
  _barrier();
  while (_atomic_xchg(&print_lock, 1) == 1);
  printf("sum = %d atomic_sum = %d\n", sum, atomic_sum);
  printf("Finalize CPU ID: %d\n", _cpu());
  _atomic_xchg(&print_lock, 0);
  while(1);
}
