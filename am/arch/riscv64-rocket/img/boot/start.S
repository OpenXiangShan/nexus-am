.section entry, "ax"
.globl _start
.type _start, @function

.extern fdt

_start:
  la sp, stack_end
  la t0, fdt
  sd x11, (t0)
  tail _trm_init

stack:
  .skip 0x4000
stack_end:
