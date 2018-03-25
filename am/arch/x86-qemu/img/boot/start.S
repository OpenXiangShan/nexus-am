#include "asm.h"
.code16
.globl _start
_start:
  cli

  xorw    %ax, %ax
  movw    %ax, %ds
  movw    %ax, %es
  movw    %ax, %ss

# Set a 640 x 480 x 32 video mode
  mov $0x4f01, %ax
  mov $0x0112, %cx
  mov $0x4000, %di
  int $0x10

  mov $0x4f02, %ax
  mov $0x4112, %bx
  int $0x10

  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE, %eax
  movl    %eax, %cr0
  ljmp    $GDT_ENTRY(1), $start32

.code32
start32:
  movw    $GDT_ENTRY(2), %ax
  movw    %ax, %ds
  movw    %ax, %es
  movw    %ax, %ss

  movl    (0x7020), %esp # multi-core hack
                         # M[0x7020] is the core's bootstrap stack offset
  addl    $0x2000, %esp
  call    bootmain

# GDT
.p2align 2
gdt:
  SEG_NULLASM
  SEG_ASM(0xA, 0x0, 0xffffffff)
  SEG_ASM(0x2, 0x0, 0xffffffff)

gdtdesc:
  .word   (gdtdesc - gdt - 1)
  .long   gdt