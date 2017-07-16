#include <npc.h>

.globl _start
.type _start, function
.globl _ex_entry
.type _ex_entry, function
 
_start:
  la $sp, _stack_pointer
  #addiu $1, $zero, 1
  #jr $1
  #li $1, 0x3611
  #li $2, 0x1234
  #addiu $1, $1, 0x20
  #lw $2, 0x0($1)
 break
 j _trm_init

.org EX_ENTRY
_ex_entry:
  j _trap_entry
