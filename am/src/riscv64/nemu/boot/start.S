.section entry, "ax"
.globl _start
.type _start, @function

// copied from riscv-tests
#define MSTATUS_FS          0x00006000
#define RVTEST_FP_ENABLE                                                \
  li a0, MSTATUS_FS & (MSTATUS_FS >> 1);                                \
  csrs mstatus, a0;                                                     \
  csrwi fcsr, 0

_start:
  RVTEST_FP_ENABLE
  mv s0, zero
  la sp, _stack_pointer
  jal _trm_init
