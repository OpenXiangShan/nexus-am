.code32

#define PML4_ADDR_TO_ENTRY_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_ADDR_TO_ENTRY_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define TWO_MEGABYTES_SHIFT 21
#define TWO_MEGABYTES (1 << TWO_MEGABYTES_SHIFT)
#define KERNEL_BASE 0x0

.section .bss
// bootstrap page table
.comm pml4, 0x1000, 0x1000
.comm low_pdpt, 0x1000, 0x1000
.comm high_pdpt, 0x1000, 0x1000
.comm low_page_directory_table, 0x1000, 0x1000
.comm high_page_directory_table, 0x1000, 0x1000

.section .text
.global _start
.type _start, @function
_start:
  movl $low_pdpt, %eax
  addl $3, %eax
  movl %eax, pml4 + (PML4_ADDR_TO_ENTRY_INDEX(KERNEL_BASE) * 8)
  movl $high_pdpt, %eax
  addl $3, %eax
  movl %eax, pml4 + (PML4_ADDR_TO_ENTRY_INDEX(KERNEL_BASE) * 8)
  movl $low_page_directory_table, %eax
  addl $3, %eax
  movl %eax, low_pdpt + (PDPT_ADDR_TO_ENTRY_INDEX(KERNEL_BASE) * 8)
  movl $high_page_directory_table, %eax
  addl $3, %eax
  movl %eax, high_pdpt + (PDPT_ADDR_TO_ENTRY_INDEX(KERNEL_BASE) * 8)
  mov $0, %ecx
  movl $_end, %esi
  shrl $21, %esi
  addl $1, %esi
page_directory_table_loop:
  movl $200000, %eax
  mul %ecx
  or $0x83, %eax
  movl %eax, low_page_directory_table(, %ecx, 8)
  movl %eax, high_page_directory_table(, %ecx, 8)

  inc %ecx
  cmp %esi, %ecx
  jne page_directory_table_loop  // if not equal redo loop

.align 128
  movl $pml4, %eax
  movl %eax, %cr3     // %cr3 = PML4 base
  movl $0x20, %eax
  movl %eax, %cr4     // %cr4.PAE = 1
  movl $0xc0000080, %ecx
  rdmsr
  or $0x100, %eax
  wrmsr // %msr.LME = 1.
  movl $0x80000011, %eax
  movl %eax, %cr0    // %cr0.PG = 1; %cr0.PE = 1;
  lgdt gdt_ptr + KERNEL_BASE // set a gdt
  ljmp $8, $_start64 + KERNEL_BASE
  cli
  hlt

gdt_ptr:
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

.code64

.global _start64
.type _start64, @function
_start64:
  movw $0, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss

  movq $0x4000, %rsp
  movq $0x7e00, %rdi
  call _start_c

  cli
  hlt
