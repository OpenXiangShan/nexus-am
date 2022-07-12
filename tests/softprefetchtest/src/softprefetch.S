# See LICENSE for license details.

#*****************************************************************************
# softprefetch.S
#-----------------------------------------------------------------------------
#
# Test that whether the cpu sppourt the softprefetch instruciton.
#

#include "encoding.h"
#include "vm_utils.h"

.global do_init

do_init:
  nop

basic_inst_test:
  li a2,  0x008000c000
  li a3,  0x008000cf00
  li a5,  0x0000000080 //128 times
  li a6,  0x0000000001
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  delay_time_1:
    sub a5, a5, a6 
    bnez a5, delay_time_1
  lw s2, 32(a2)
  sw a6, 32(a3) 
  li a5,  0x0000000080 //128 times 
  delay_time_2:
    sub a5, a5, a6 
    bnez a5, delay_time_2

prefetch_then_read:
  li a2,  0x008000c010
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  lw s2, 32(a2)

prefetch_then_write:
  li a3,  0x008000cf10
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  sw a6, 32(a3)

read_then_prefetch:
  li a2,  0x008000c020
  lw s2, 32(a2)
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2

write_then_prefetch:
  li a3,  0x008000cf20
  sw a6, 32(a3)
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 

prefetch_storm:
  li a2,  0x008000c000
  li a3,  0x008000cf00
  li a5,  0x0000000080 //128 times
  li a6,  0x0000000001
storm_start:
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  sub a5, a5, a6 
  bnez a5, storm_start

prefetch_from_illegal_addr:
  // register exception_handler 
  la a0, exception_handler_entry
  csrw mtvec, a0 //register exception_handler
  csrw stvec, a0 //register exception_handler
  // torture softprefetch with strange addrs
  li a2, 0x0
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  li a2, 0x23333
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  li a2, 0x88888888
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  li a2, 0x40000050
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  li a3, 0x0
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  li a3, 0x23333
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  li a3, 0x88888888
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  li a3, 0x40000050
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 

prefetch_with_vm_enabled:
  INIT_PMP
enable_vm:
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

  # Set up data page
  la t0, data_page
  srl t0, t0, RISCV_PGSHIFT - PTE_PPN_SHIFT
  ori t0, t0, PTE_V | PTE_X | PTE_A | PTE_D | PTE_W | PTE_R
  sd t0, page_table_3 + 8, t1

  # Turn on VM

  li a0, (SATP_MODE & ~(SATP_MODE<<1)) * SATP_MODE_SV39
  la a1, page_table_1
  srl a1, a1, RISCV_PGSHIFT
  or a1, a1, a0
  csrw sptbr, a1
  sfence.vma

  # Enter supervisor mode and make sure correct page is accessed
  la a2, exception_handler_entry
  csrwi mepc, 0
  li a1, ((MSTATUS_MPP & ~(MSTATUS_MPP<<1)) * PRV_S)
  csrs mstatus, a1
  mret // goto code_page

exception_handler_entry:
  // soft prefetch should not raise exception 
  // if exception is reported, it means soft prefetch f***ed up
  j failure 

success:
  li a0, 0
  .word 0x0000006b

failure:
  li a0, 1
  .word 0x0000006b

.data

.align 12
page_table_1: 
  .dword 0

.align 12
page_table_2: 
  .dword 0

.align 12
page_table_3:
  .dword 0

.align 12
code_page: # smode softprefetch test workload
  # prefetch from valid addr
  la a2, 0x1000
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  la a3, 0x1010
  addi a3, a3, 0x10
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  lw s2, 32(a2)
  lw s2, 32(a3)
  # prefetch from invalid addr
  la a2, data_page # invalid addr in smode
  la a3, data_page # invalid addr in smode
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  # no exception should be reported
  # add some nop to trigger difftest of $a0
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  li a0, 0
  .word 0x0000006b // report success

.align 16
data_page: 
  .dword 0