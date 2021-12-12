# See LICENSE for license details.

#*****************************************************************************
# softprefetch.S
#-----------------------------------------------------------------------------
#
# Test that whether the cpu sppourt the softprefetch instruciton.
#

#include "encoding.h"

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
  j success

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

