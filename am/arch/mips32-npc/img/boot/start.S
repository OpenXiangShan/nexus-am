.section loader, "x"
.globl _start
.type _start, @function
.globl _ex_entry
.type _ex_entry, @function

_start:
  li $sp, 0x17fffffc
  la $k0, _trm_init
  jr $k0

.org 0x20
_ex_entry:
  la $k0, _trap_entry
  jr $k0
