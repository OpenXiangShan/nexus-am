#include <amtest.h>

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