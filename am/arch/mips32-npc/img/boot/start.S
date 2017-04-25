.globl _start
.type _start, @function
.globl _qemu
.type _qemu, @function

_start:
  li $sp, 0x0000fffc
  jal main
  nop

_qemu:
  nop
  jal main
  nop
