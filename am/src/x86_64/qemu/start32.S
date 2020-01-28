#include <x86.h>

.globl _start
_start:
  pushl $ARG_ADDR
  pushl $0
  jmp _start_c
