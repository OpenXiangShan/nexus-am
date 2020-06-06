.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mv s0, zero

  # SDRAM initialization
  li t0, 0x41400008
  sw zero, (t0)

  la sp, _stack_pointer
  jal _trm_init
