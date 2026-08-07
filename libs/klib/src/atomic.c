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

/* Keep the M-mode lock ABI and interrupt behavior used during boot. */
void lock(volatile uint64_t *addr) {
  asm volatile("csrci mstatus, 0x8");
  while(compare_and_swap(addr, 0, 1));
}

void release(volatile uint64_t *addr) {
  *addr = 0;
  asm volatile("fence");
  asm volatile("csrsi mstatus, 0x8");
}

#define SSTATUS_SIE (1UL << 1)

/* sstatus.SIE is the supervisor-level global interrupt-enable bit. */
uintptr_t s_lock_irqsave(volatile uint64_t *addr) {
  uintptr_t status;
  asm volatile(
    "csrrc %0, sstatus, %1"
    : "=r"(status)
    : "r"(SSTATUS_SIE)
    : "memory"
  );
  while (compare_and_swap(addr, 0, 1)) {
  }
  asm volatile("fence r, rw" ::: "memory");
  return status;
}

void s_release_irqrestore(volatile uint64_t *addr, uintptr_t status) {
  asm volatile("fence rw, w" ::: "memory");
  *addr = 0;
  if (status & SSTATUS_SIE) {
    asm volatile("csrs sstatus, %0" : : "r"(SSTATUS_SIE) : "memory");
  }
}
