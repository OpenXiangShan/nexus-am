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
basic_csr_rw_case1:
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
basic_csr_rw_case2:
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

#-----------------------------------------------------------------------------
# Test 2: basic cache op flow
#-----------------------------------------------------------------------------

# dcache data write/read op: loop test

# write 0x12345678, 0x0, 0x0 ... to dcache index 0 way 0
basic_flow_case1:
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

#-----------------------------------------------------------------------------
# Test 3: dcache data read/write
#-----------------------------------------------------------------------------

# write 0x0, 0x1, 0x2, 0x3 ... to dcache index 1 way 1
dcache_data_case1:
  # write data cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x0
  csrw cop_data_0, a0
  li a0, 0x1
  csrw cop_data_1, a0
  li a0, 0x2
  csrw cop_data_2, a0
  li a0, 0x3
  csrw cop_data_3, a0
  li a0, 0x4
  csrw cop_data_4, a0
  li a0, 0x5
  csrw cop_data_5, a0
  li a0, 0x6
  csrw cop_data_6, a0
  li a0, 0x7
  csrw cop_data_7, a0
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

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
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

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
  li a2, 0x0
  bne a1, a2, failure
  csrr a1, cop_data_1
  li a2, 0x1
  bne a1, a2, failure
  csrr a1, cop_data_2
  li a2, 0x2
  bne a1, a2, failure
  csrr a1, cop_data_3
  li a2, 0x3
  bne a1, a2, failure
  csrr a1, cop_data_4
  li a2, 0x4
  bne a1, a2, failure
  csrr a1, cop_data_5
  li a2, 0x5
  bne a1, a2, failure
  csrr a1, cop_data_6
  li a2, 0x6
  bne a1, a2, failure
  csrr a1, cop_data_7
  li a2, 0x7
  bne a1, a2, failure

#-----------------------------------------------------------------------------
# Test 4: dcache tag read/write
#-----------------------------------------------------------------------------

# write 0x233 to dcache index 1 way 1 tag, then read it
dcache_tag_case1:
  # write data cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x233
  csrw cop_tag_data, a0
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_WRITE_TAG
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure

  # read data cache tag using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_READ_TAG
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure
  csrr a1, cop_tag_data
  li a2, 0x233
  bne a1, a2, failure

# write 0x87654321 to dcache index 1 way 1 tag, then read it
dcache_tag_case2:
  # write data cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x87654321
  csrw cop_tag_data, a0
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_WRITE_TAG
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure

  # read data cache tag using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_READ_TAG
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure
  csrr a1, cop_tag_data
  li a2, 0x87654321
  beq a1, a2, failure

#-----------------------------------------------------------------------------
# Test 5: dcache ecc read/write
#-----------------------------------------------------------------------------

# write 0x3 to dcache index 1 way 1 bank 1 data ecc, then read it
dcache_ecc_case1:
  # write data cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x3
  csrw cop_data_ecc, a0
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_WRITE_DATA_ECC
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure

  # read data cache tag using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_READ_DATA_ECC
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure
  csrr a1, cop_data_ecc
  li a2, 0x3
  bne a1, a2, failure

# write 0x3 to dcache index 1 way 1 bank 1 tag ecc, then read it
dcache_ecc_case2:
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x3
  csrw cop_tag_ecc, a0
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_WRITE_TAG_ECC
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure

  # read data cache tag using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_DCACHE
  csrw cop_level, a2
  li a2, 0x1
  csrw cop_way, a2
  li a2, 0x1
  csrw cop_index, a2

  # send cache op request
  li a2, COP_READ_TAG_ECC
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure
  csrr a1, cop_tag_ecc
  li a2, 0x3
  bne a1, a2, failure

#-----------------------------------------------------------------------------
# Test 6: dcache op mixed with load/store
#-----------------------------------------------------------------------------

# Note: we used hard coded paddr TODO to test dcache op

# write a cache line, then fence to ensure it enters dcache
dcache_mixed_step1:

# load from that address to load it into dcache
dcache_mixed_step2:

# use cache op to locate that cache line (get way by reading tag) 
dcache_mixed_step3:

# use cache op to read data from that cache line
dcache_mixed_step4:

# use cache op to read tag from that cache line
dcache_mixed_step5:

# use cache op to read ecc from that cache line
dcache_mixed_step6:

# use cache op to write 0x0, 0x1, 0x2, 0x3 ... to that cache line
dcache_mixed_step7:

# load from that address, check result
dcache_mixed_step8:

#-----------------------------------------------------------------------------
# Test 7: insert dcache error
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# test finished
#-----------------------------------------------------------------------------

  j success
  
#-----------------------------------------------------------------------------
# misc
#-----------------------------------------------------------------------------


