#-----------------------------------------------------------------------------
# workload.S
#-----------------------------------------------------------------------------
#
# Test customized inst/data cache control insts
#
#-----------------------------------------------------------------------------

#include "encoding.h"
#include "custom_encoding.h"
#include "cacheop_utils.h"

.global workload
workload:
  la a0, my_mtvec_handler
  csrw mtvec, a0
  nop

#-----------------------------------------------------------------------------
# 1. wait for at least 4096 cycles to let hardware make ecc error
#-----------------------------------------------------------------------------
  li t1, 0x0
  li t0, 256
loop:
  addi t1, t1, 1
  bne t0, t1, loop

  li a0, 0x80002000
  ld a0, (a0)

#-----------------------------------------------------------------------------
# 2. trigger dcache ecc error
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# test finished
#-----------------------------------------------------------------------------
  ret

my_mtvec_handler:
#  li a0, 0x80002000
#  ld a0, (a0)
#  li a1, 0
#  bne a0, a1, fail_22

  csrr a0, mcause
  li a1, 5
  beq a0, a1, success_22

fail_22:
  li a0, 1
  .word 0x0005006b

success_22:
  li a0, 0
  .word 0x0005006b

.align 12
data_page_00: .dword 0
.align 12