#include "encoding.h"

#define TESTNUM gp

.global do_alias_jit_init
do_alias_jit_init:

  la t0, 1f;
  csrw mtvec, t0;
  /* Set up a PMP to permit all accesses */
  li t0, (1 << (31 + (__riscv_xlen / 64) * (53 - 31))) - 1;
  csrw pmpaddr0, t0;
  li t0, PMP_NAPOT | PMP_R | PMP_W | PMP_X;
  csrw pmpcfg0, t0;
  .align 2;

  # Set up intermediate page tables

  la t0, page_table_3
  srl t0, t0, RISCV_PGSHIFT - PTE_PPN_SHIFT
  ori t0, t0, PTE_V
  sd t0, page_table_2, t1

  la t0, page_table_2
  srl t0, t0, RISCV_PGSHIFT - PTE_PPN_SHIFT
  ori t0, t0, PTE_V
  sd t0, page_table_1, t1

  # Set up code page

  la t0, code_page
  srl t0, t0, RISCV_PGSHIFT - PTE_PPN_SHIFT
  ori t0, t0, PTE_V | PTE_X | PTE_A
  sd t0, page_table_3 + 0, t1

  # Set up 2 data page mapped to the same physical page
  la t0, data_page
  srl t0, t0, RISCV_PGSHIFT - PTE_PPN_SHIFT
  ori t0, t0, PTE_V | PTE_X | PTE_A | PTE_D | PTE_W | PTE_R
  sd t0, page_table_3 + 8, t1

  la t0, data_page
  srl t0, t0, RISCV_PGSHIFT - PTE_PPN_SHIFT
  ori t0, t0, PTE_V | PTE_X | PTE_A | PTE_D | PTE_W | PTE_R
  sd t0, page_table_3 + 16, t1

  # Turn on VM

  li a0, (SATP_MODE & ~(SATP_MODE<<1)) * SATP_MODE_SV39
  la a1, page_table_1
  srl a1, a1, RISCV_PGSHIFT
  or a1, a1, a0
  csrw sptbr, a1
  sfence.vma

  # Write code to code_page

  la t0, data_page
  la t1, 0x000025b7 # la a1, 0x2000
  sw t1, (t0)
  la t1, 0x0005a503 # lw a0, 0(a1)
  sw t1, 4(t0)
  la t1, 0x12000073 # sfence.vma
  sw t1, 8(t0)
  la t1, 0x0000051b # la a0, 0x0
  sw t1, 12(t0)
  la t1, 0x0000006b # _halt
  sw t1, 16(t0)
  sfence.vma

  # Enter supervisor mode and make sure correct page is accessed

  la a2, 1f
  la t3, 0x1000
  csrw mepc, t3
  li a1, (MSTATUS_MPP * PRV_S)
  csrc mstatus, a1
  li a1, ((MSTATUS_MPP & ~(MSTATUS_MPP<<1)) * PRV_S)
  csrs mstatus, a1
  mret

  .align 2
  .global mtvec_handler_jit
mtvec_handler_jit:
  csrr t0, mcause
  add t0, t0, -CAUSE_STORE_PAGE_FAULT
  bnez t0, fail

  jr a2

fail:
1:
  la a0, -1
  .word 0x0000006b
  j 1b

.data
.align 12
page_table_1: .dword 0
.align 12
page_table_2: .dword 0
.align 12
page_table_3: .dword 0
.align 13
code_page:
  la a0, 0xbeef
  la a1, 0x1000
  sw a0, (a1)
  sfence.vma
  la a0, 0xdead
  la a1, 0x2000
  sw a0, (a1)
  sfence.vma

  la a1, 0x1000
  lw a0, (a1)

# add some nop to trigger difftest of $a0
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop

  la a0, 0
  .word 0x0000006b

.align 12
data_page: .dword 0
