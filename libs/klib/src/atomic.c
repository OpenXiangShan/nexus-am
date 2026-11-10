#include "klib.h"
#include <klib-macros.h>

size_t compare_and_swap(volatile size_t* addr, size_t old_val, size_t new_val) {
  size_t check = 0;
  size_t value = 0;
  #if __riscv_xlen == 64
  asm volatile (
    "lr.d %[value], (%[addr]);"
    : [value]"=r"(value)
    : [addr]"p"(addr)
  );
  #else
  asm volatile (
    "lr.w %[value], (%[addr]);"
    : [value]"=r"(value)
    : [addr]"p"(addr)
  );
  #endif

  if (value != old_val) return 1;
  #if __riscv_xlen == 64
  asm volatile (
    "sc.d %[check], %[write], (%[addr]);"
    : [check]"=r"(check)
    : [write]"r"(new_val), [addr]"p"(addr)
  );
  #else
  asm volatile (
    "sc.w %[check], %[write], (%[addr]);"
    : [check]"=r"(check)
    : [write]"r"(new_val), [addr]"p"(addr)
  );
  #endif
  return check;
}

void lock(volatile size_t *addr) {
  asm volatile("csrci mstatus, 0x8");
  while(compare_and_swap(addr, 0, 1));
}

void release(volatile size_t *addr) {
  *addr = 0;
  asm volatile("fence");
  asm volatile("csrsi mstatus, 0x8");
}
