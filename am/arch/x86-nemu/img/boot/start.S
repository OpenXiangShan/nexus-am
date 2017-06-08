.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mov $0, %ebp
  mov $_stack_pointer - 16, %esp       # keep the 'bt' command happy
  call _trm_init

