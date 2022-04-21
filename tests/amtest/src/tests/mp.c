#include <amtest.h>

/*
 * Two build mp test, try `make ARCH=riscv64-xs-dual mainargs='m2'`
 */

static volatile intptr_t sum;
static volatile intptr_t print_lock = 0;

void __am_uartlite_putchar(char ch);

void mp_print() {
  _mpe_wakeup(1);
  while (_atomic_xchg(&print_lock, 1) == 1);
  printf("My CPU ID is %d, nrCPU is %d\n", _cpu(), _ncpu());
  _atomic_xchg(&print_lock, 0);
  _atomic_add(&sum, 1);
}

void finalize() {
  while (_atomic_xchg(&print_lock, 1) == 1);
  printf("Finalize CPU ID: %d\n", _cpu());
  _atomic_xchg(&print_lock, 0);
  while(1);
}
