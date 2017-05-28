.globl _start
.type _start, @function
.globl _qemu
.type _qemu, @function
.extern _trm_init

_start:
  li $sp, 0x17fffffc
  jal _trm_init

_qemu:
  nop
  jal _trm_init
