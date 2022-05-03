#include <amtest.h>
#include <pmp.h>
#include <xsextra.h>

/*
 * Note that to be able to run this test, PMP should be set up before AM jump to supervisor
 * You may find related initialzation code in __am_init_cte64()
 */

#define PMP_1

#define EXCEPTION_STORE_ACCESS_FAULT 7

uint64_t right_store_access_fault_addr = 0;

_Context* store_access_fault_handler(_Event* ev, _Context *c) {
  // printf("store access fault triggered\n");
  uint64_t stval = csr_read(stval);
  if (right_store_access_fault_addr == stval) {
    // printf("store access fault hit addr %x\n", right_store_access_fault_addr);
    _halt(0);
  } else {
    // printf("store access fault addr mismatch: right %x, wrong %x, pc %x\n", 
    //   right_store_access_fault_addr,
    //   stval,
    //   c->sepc
    // );
    _halt(1); // something went wrong
  }
  return c;
}

void pmp_test() {
  register_handler(EXCEPTION_STORE_ACCESS_FAULT, &store_access_fault_handler);
  printf("start pmp test\n");
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
  #ifdef PMP_1
  volatile int *a = (int *)(0x90000040UL);
  *a = 1; // should trigger a fault
  #endif
  #ifdef PMP_2
  int *b = (int *)(0xa0000000UL);
  *b = 1; // should not trigger a fault
  #endif
  #ifdef PMP_3
  int *c = (int *)(0xb00000040UL);
  *c = 1; // should trigger a fault
  #endif
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  right_store_access_fault_addr = 0;
  int *b = (int *)(0x2030000000UL);
  *b = 1; // should not trigger a fault
  right_store_access_fault_addr = 0x2010000040UL;
  volatile int *a = (int *)(0x2010000040UL);
  *a = 1; // should trigger a fault
#else
  // invalid arch
  _halt(1);
#endif
}