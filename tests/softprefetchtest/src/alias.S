# See LICENSE for license details.

#*****************************************************************************
# softprefetch-alias.S
#-----------------------------------------------------------------------------
#
# Test that whether the cpu sppourt the softprefetch instruciton.
#

#include "encoding.h"

#define TESTNUM gp

.global do_init
do_init:
  li a2,  0x008000c000
  li a3,  0x008000cf00
  li a5,  0x0000000080 //128 times
  li a6,  0x0000000001
  la a4, code_page
  jr a4

.data
.align
code_page:
  .word 0x02166013 //prefetch.r 0x10(a2), offset is 32, base register is a2
  .word 0x0236e013 //prefetch.w 0x10(a3), offset is 32, base register is a3 
  delay_time_1:
    sub a5, a5, a6 
    bnez a5, delay_time_1
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  lw s2, 32(a2)
  sw a6, 32(a3) 
  li a5,  0x0000000080 //128 times 
  delay_time_2:
    sub a5, a5, a6 
    bnez a5, delay_time_2
  .word 0x0000006b

.align 12
data_page: .dword 0
