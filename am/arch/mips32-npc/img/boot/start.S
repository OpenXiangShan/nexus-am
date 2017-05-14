.globl _start
.type _start, @function
.globl _qemu
.type _qemu, @function

_start:
  li $sp, 0xf7fffffc
  jal main

_qemu:
  nop
  jal main
