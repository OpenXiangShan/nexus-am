#include <amtest.h>

/*
 * Note that to be able to run this test, PMP should be set up before AM jump to supervisor
 * You may find related initialzation code in __am_init_cte64()
 */

#define PMP_1

void pmp_test() {
  printf("start pmp test\n");
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
  asm volatile("ebreak");
}