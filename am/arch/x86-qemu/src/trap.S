#include <x86.h>

#----|-----entry------|-errorcode-|-----id-----|-handler-|
.globl vec0;    vec0:   pushl $0;  pushl    $0; jmp trap
.globl vec1;    vec1:   pushl $0;  pushl    $1; jmp trap
.globl vec2;    vec2:   pushl $0;  pushl    $2; jmp trap
.globl vec3;    vec3:   pushl $0;  pushl    $3; jmp trap
.globl vec4;    vec4:   pushl $0;  pushl    $4; jmp trap
.globl vec5;    vec5:   pushl $0;  pushl    $5; jmp trap
.globl vec6;    vec6:   pushl $0;  pushl    $6; jmp trap
.globl vec7;    vec7:   pushl $0;  pushl    $7; jmp trap
.globl vec8;    vec8:              pushl    $8; jmp trap
.globl vec9;    vec9:   pushl $0;  pushl    $9; jmp trap
.globl vec10;   vec10:             pushl   $10; jmp trap
.globl vec11;   vec11:             pushl   $11; jmp trap
.globl vec12;   vec12:             pushl   $12; jmp trap
.globl vec13;   vec13:             pushl   $13; jmp trap
.globl vec14;   vec14:             pushl   $14; jmp trap
.globl vecsys; vecsys:  pushl $0;  pushl $0x80; jmp trap
.globl irq0;     irq0:  pushl $0;  pushl   $32; jmp trap
.globl irq1;     irq1:  pushl $0;  pushl   $33; jmp trap
.globl irq14;   irq14:  pushl $0;  pushl   $46; jmp trap
.globl irqall; irqall:  pushl $0;  pushl   $-1; jmp trap

trap:
  cli
  pushl %ds
  pushl %es
  pushal

  movw $KSEL(SEG_KDATA), %ax
  movw %ax, %ds
  movw %ax, %es

  pushl %esp
  call irq_handle
  addl $4, %esp

  popal
  popl %es
  popl %ds
  addl $8, %esp

  iret
