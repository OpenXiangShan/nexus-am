.section entry, "ax"
.globl _start
.type _start, @function

#define MSTATUS_FS 0x00006000

_start:
  mv s0, zero
  li a0, MSTATUS_FS & (MSTATUS_FS >> 1)
  csrs mstatus, a0
  csrwi fcsr, 0
  
#define STKSHIFT 17  // 128KB for each stack && TLS
  csrr a0, mhartid
  la tp, _stack_top
  add sp, a0, 1
  sll sp, sp, STKSHIFT
  add sp, sp, tp
  sll a2, a0, STKSHIFT
  add tp, tp, a2
  
  jal _trm_init

