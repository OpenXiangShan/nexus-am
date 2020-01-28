#include <x86.h>

.code32
.globl _start
_start:
  #define PML4 0x1000
  #define PDPT 0x2000

  movl $PDPT + 3, %eax
  cmpl (PML4), %eax
  je .long_mode_init

  movl $PDPT + 3, %eax
  movl %eax, (PML4)

  movl $0, %ecx
  movl $512, %esi //  512 pages
                  //   |
.loop:            //   x
  movl %ecx, %eax //   |
  shll $30, %eax  //   |
  orl $0x83, %eax // 1 GiB page
  movl %eax, PDPT(, %ecx, 8)

  movl %ecx, %eax
  shrl $2, %eax
  movl %eax, PDPT + 4(, %ecx, 8)

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
  wrmsr               // %msr.LME = 1.
  movl $0x80000011, %eax
  movl %eax, %cr0     // %cr0.PG = 1, %cr0.PE = 1
  lgdt gdt_ptr        // bootstrap GDT
  ljmp $8, $_start64  // should not return

.code64
_start64:
  movw $0, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %ss
  movw %ax, %fs
  movw %ax, %gs

  movq $ARG_ADDR, %rdi
  pushq $0
  jmp _start_c

.align 16
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
