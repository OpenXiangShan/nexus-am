#include "trap.h"

static volatile unsigned int value __attribute__((aligned(4)));

static unsigned long amomin_w(volatile unsigned int *addr, unsigned long operand) {
  unsigned long old_value;
  asm volatile(
    "amomin.w %0, %2, (%1)"
    : "=&r"(old_value)
    : "r"(addr), "r"(operand)
    : "memory"
  );
  return old_value;
}

static unsigned long amomax_w(volatile unsigned int *addr, unsigned long operand) {
  unsigned long old_value;
  asm volatile(
    "amomax.w %0, %2, (%1)"
    : "=&r"(old_value)
    : "r"(addr), "r"(operand)
    : "memory"
  );
  return old_value;
}

static unsigned long amominu_w(volatile unsigned int *addr, unsigned long operand) {
  unsigned long old_value;
  asm volatile(
    "amominu.w %0, %2, (%1)"
    : "=&r"(old_value)
    : "r"(addr), "r"(operand)
    : "memory"
  );
  return old_value;
}

static unsigned long amomaxu_w(volatile unsigned int *addr, unsigned long operand) {
  unsigned long old_value;
  asm volatile(
    "amomaxu.w %0, %2, (%1)"
    : "=&r"(old_value)
    : "r"(addr), "r"(operand)
    : "memory"
  );
  return old_value;
}

int main() {
  const unsigned long all_ones_word = 0x00000000ffffffffUL;
  unsigned long old_value;

  value = 1;
  old_value = amomin_w(&value, all_ones_word);

  nemu_assert(old_value == 1);
  nemu_assert(value == 0xffffffffU);

  value = 0xffffffffU;
  old_value = amomax_w(&value, 1);

  nemu_assert(old_value == 0xffffffffffffffffUL);
  nemu_assert(value == 1);

  value = 0x80000000U;
  old_value = amominu_w(&value, all_ones_word);

  nemu_assert(old_value == 0xffffffff80000000UL);
  nemu_assert(value == 0x80000000U);

  value = 0x80000000U;
  old_value = amomaxu_w(&value, all_ones_word);

  nemu_assert(old_value == 0xffffffff80000000UL);
  nemu_assert(value == 0xffffffffU);

  return 0;
}
