#include <x86.h>

.globl _start
.type _start, @function
_start:

#ifndef __x86_64__
  // 32-bit code
  pushl $0x7e00
  pushl $0
  jmp _start_c

#else
  // 64-bit code
.code32
  #define PML4 0x1000
  #define PDPT 0x2000
  #define PDT  0x3000
  #define PDTH 0x4000

  movl $PDPT + 3, %eax
  cmpl (PML4), %eax
  je .long_mode_init

  movl %eax, (PML4)
  movl $PDT + 3, %eax
  movl %eax, (PDPT)
  movl $PDTH + 3, %eax
  movl %eax, (24 + PDPT)

  movl $0, %ecx
  movl $512, %esi

.loop:
  movl $0x200000, %eax
  mul %ecx
  orl $0x83, %eax // 2 MiB page
  movl %eax, PDT(, %ecx, 8)
  
  orl $0xc0000000, %eax
  movl %eax, PDTH(, %ecx, 8)

  inc %ecx
  cmp %esi, %ecx
  jne .loop

.long_mode_init:
  movl $PML4, %eax
  movl %eax, %cr3     // %cr3 = PML4 base
  movl $0x80000021, %eax
  movl %eax, %cr4     // %cr4.PAE = 1
  movl $0xc0000080, %ecx
  rdmsr
  orl $0x100, %eax
  wrmsr // %msr.LME = 1.
  movl $0x80000011, %eax
  movl %eax, %cr0    // %cr0.PG = 1; %cr0.PE = 1;
  lgdt gdt_ptr // set a gdt
  ljmp $8, $_start64
  cli
  hlt

gdt_ptr:
  .word gdt64_end - gdt64_begin - 1
  .quad gdt64_begin

gdt64_begin:
  .long 0x00000000 # 0: null desc
  .long 0x00000000
  .long 0x00000000 # 1: Code, R/X, Nonconforming
  .long 0x00209800
  .long 0x00000000 # 2: Data, R/W, Expand Down
  .long 0x00009800
gdt64_end:

.code64

_start64:
  movw $0, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss
  movw %ax, %fs
  movw %ax, %gs

  movq $0x7e00, %rdi
  pushq $0
  jmp _start_c

#endif
