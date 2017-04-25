.globl _start
.type _start, @function
.globl _qemu
.type _qemu, @function

_start:
  li $sp, 0x00017ffc
  jal main

.globl _qemu
_qemu:
  nop
  jal main
