#include "klib.h"
#include <klib-macros.h>

uint64_t compare_and_swap(volatile uint64_t* addr, uint64_t old_val, uint64_t new_val) {
  uint64_t check = 0;
  uint64_t value = 0;
  asm volatile (
    "lr.d %[value], (%[addr]);"
    : [value]"=r"(value)
    : [addr]"p"(addr)
  );
  if (value != old_val) return 1;
  asm volatile (
    "sc.d %[check], %[write], (%[addr]);"
    : [check]"=r"(check)
    : [write]"r"(new_val), [addr]"p"(addr)
  );
  return check;
}

void lock(volatile uint64_t *addr) {
  asm volatile("csrci mstatus, 0x8");
  while(compare_and_swap(addr, 0, 1));
}

void release(volatile uint64_t *addr) {
  *addr = 0;
  asm volatile("fence");
  asm volatile("csrsi mstatus, 0x8");
}