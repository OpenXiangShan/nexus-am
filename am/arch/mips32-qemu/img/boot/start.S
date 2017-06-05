#include <mips32-qemu.h>

.globl _start
.type _start, function
.globl _ex_entry
.type _ex_entry, function

_start:
  la $sp, _stack_pointer
  j _trm_init

.org EX_ENTRY
_ex_entry:
  j _ex_entry
