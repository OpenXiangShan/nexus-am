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
  nop

#-----------------------------------------------------------------------------
# Test 1: cache op ctrl csr read / write
#-----------------------------------------------------------------------------

# basic csr read/write
case1:
  li a0, 0
  csrw cop_finish, a0
  csrr a1, cop_finish
  bne a0, a1, failure

  li a0, 0x12345678
  csrw cop_level, a0
  csrr a1, cop_level
  bne a0, a1, failure

  li a0, 0x12345678
  csrw cop_data_0, a0
  csrr a1, cop_data_0
  bne a0, a1, failure

# basic dcache data write op
case2:
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x12345678
  csrw cop_data_0, a0
  li a2, COP_ID_DCACHE
  csrw cop_level, a2

  # send cache op resuest
  li a2, COP_WRITE_DATA
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # jal case_passed

#-----------------------------------------------------------------------------
# Test 2: basic cache op flow
#-----------------------------------------------------------------------------

# dcache data write/read op: loop test
case3:
  # write data cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x12345678
  csrw cop_data_0, a0
  li a2, COP_ID_DCACHE
  csrw cop_level, a2

  # send cache op request
  li a2, COP_WRITE_DATA
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure

  # read data cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_DCACHE
  csrw cop_level, a2

  # send cache op request
  li a2, COP_READ_DATA
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure
  csrr a1, cop_data_0
  li a2, 0x12345678
  bne a1, a2, failure

  # basic flow check passed
  # jal case_passed

#-----------------------------------------------------------------------------
# Test 3: dcache tag read/write
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Test 3: dcache data read/write
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Test 4: dcache ecc read/write
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Test 5: icache tag read/write
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Test 6: icache data read/write
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# test finished
#-----------------------------------------------------------------------------

  j success
  
#-----------------------------------------------------------------------------
# misc
#-----------------------------------------------------------------------------


