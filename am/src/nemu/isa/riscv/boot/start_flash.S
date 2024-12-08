.section entry, "ax"
.globl _start
.type _start, @function

#define MSTATUS_FS    0x00006000

#define pmpaddr1      0x3b1
#define pmpaddr2      0x3b2
#define pmpcfg0       0x3a0

_start:
  mv s0, zero
  li a0, MSTATUS_FS & (MSTATUS_FS >> 1)
  csrs mstatus, a0
  csrwi fcsr, 0

  # csrr  a0, pmpcfg0
  # li    a0, (0x80000000 >> 2)
  # csrw  pmpaddr1, a0
  # li    a1, (0x90000000 >> 2)
  # csrw  pmpaddr2, a0
  # csrr  a0, pmpcfg0
  # li    a1, (0x8b << 16) // locked, top of range, +wr, -x
  # or    a0, a0, a1
  # csrw  pmpcfg0, a0

  la t0, _stack_top
  la t1, _stack_pointer
  sub t3, t1, t0
  csrr t4, mhartid
  mul t5, t3, t4
  add sp, t5, t1

  jal _trm_init