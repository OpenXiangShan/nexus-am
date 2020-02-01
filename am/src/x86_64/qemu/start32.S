#include <x86.h>
#include "x86_64-qemu.h"

.globl _start
_start:
  pushl $MAINARG_ADDR
  pushl $0
  jmp _start_c
