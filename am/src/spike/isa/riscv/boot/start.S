.section entry, "ax"
.globl _start
.type _start, @function

#define MSTATUS_FS 0x00006000

_start:
  mv s0, zero
  la sp, _stack_pointer
  li a0, MSTATUS_FS & (MSTATUS_FS >> 1)
  csrs mstatus, a0
  csrwi fcsr, 0
  jal _trm_init

.section ".tohost","aw",@progbits
.align 6
.globl tohost
tohost: .dword 0
.align 6
.globl fromhost
fromhost: .dword 0