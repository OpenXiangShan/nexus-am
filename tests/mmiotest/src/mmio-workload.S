# See LICENSE for license details.

#-----------------------------------------------------------------------------
# mmio-workload.S
#-----------------------------------------------------------------------------
#
# Test DCache MMIO insts
#

#include "encoding.h"

.global do_init
do_init:

#-----------------------------------------------------------------------------
# Test 1: mixed MMIO-store + store
#-----------------------------------------------------------------------------

  # prepare address
  li s1, 0
  lui s0, 0xdead
  lui	a4,0x40600
  lbu	a5,8(a4) # 40600008: hard coded XiangShan MMIO address
  la	a1, naive_data

loop:
  # store
  sd	s0,0(a1)
  sd	s0,8(a1)
  # mmio store
  sb	s0,4(a4)
  # store
  sd	s0,0(a1)
  sd	s0,8(a1)
  sd	s0,16(a1)
  sd	s0,24(a1)
  sd	s0,32(a1)
  sd	s0,40(a1)
  # mmio store
  sb	s0,4(a4)
  sb	s0,4(a4)
  # store
  sd	s0,48(a1)
  sd	s0,56(a1)
  sd	s0,64(a1)
  sd	s0,72(a1)
  
  addi s1,s1,1
  lui t0,10
  bne s1,t0,loop
  
#-----------------------------------------------------------------------------
# test finished
#-----------------------------------------------------------------------------

# add some nop to trigger difftest of $a0
  la a0, 0
  .word 0x0000006b
  nop
  nop
  nop
  
.data
naive_data:
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
  .word 0x0
code_page:
  # la a0, 0
  # .word 0x0000006b

.align 12
data_page: .dword 0
