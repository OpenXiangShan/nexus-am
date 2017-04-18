.globl _reset
.type _reset, @function

.globl _qemu
.type _qemu, @function

.globl _bad
.type _bad, @function

_reset:
  li $sp, 0x0001fffc
  jal bootmain
$L_GOOD:
  b $L_GOOD

_qemu:
  nop # Make address the same offset
  jal bootmain
$L_LOOP:
  b $L_LOOP

_bad:
$L_BAD:
  b $L_BAD
