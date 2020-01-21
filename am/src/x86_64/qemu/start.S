#include <x86.h>

#define KERNEL_BASE 0x00100000

.code32

.globl _start
_start:
  movl $0x1000, %edi
  movl %edi, %cr3

  movl $0x2003, (%edi)
  addl $0x1000, %edi
  movl $0x3003, (%edi)
  addl $0x1000, %edi
  movl $0x4003, (%edi)
  addl $0x1000, %edi

  movl $0x3, %ebx
  movl $512, %ecx
.fill_pt:
  movl %ebx, (%edi)
  addl $4096, %ebx
  addl $8, %edi
  loop .fill_pt
  
.align 128
  movl %cr0, %eax
  orl $(CR0_PE | CR0_PG), %eax
  movl %eax, %cr4 // enable CR4 PE and PG

  movl $0xc0000080, %ecx
  rdmsr
  orl $256, %eax
  wrmsr // enable long mode

  movl %cr0, %eax
  orl $(CR0_PE | CR0_PG), %eax
  movl %eax, %cr0 // eanble CR0 PE and PG

  lgdt gdtr64
  ljmp $8, $entry64

.align 128
entry64:
  jmp .

gdtr64:
  .word gdt64_end - gdt64_begin - 1;
  .quad gdt64_begin + KERNEL_BASE

gdt64_begin:
  .long 0xffff0000 # 0: null desc
  .long 0x00000000
  .long 0x00000000 # 1: Code, R/X, Nonconforming
  .long 0x00209800
  .long 0x00000000 # 2: Data, R/W, Expand Down
  .long 0x00009000
gdt64_end:
