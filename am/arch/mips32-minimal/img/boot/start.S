.globl _start
.type _start, @function
  
_start:
  la $sp, _stack_pointer
  j _trm_init
