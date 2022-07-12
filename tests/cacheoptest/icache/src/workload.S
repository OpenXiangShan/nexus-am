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
# Test 1: icache tag read/write
#-----------------------------------------------------------------------------

# write 0x233 to icache index 0xff bank 0 tag, then read it
icache_tag_case1:
  # write inst cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x233
  csrw cop_tag_data, a0
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 0xff # 0xff it is not used by inst
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

  # read inst cache tag using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 0xff
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

#-----------------------------------------------------------------------------
# Test 2: icache data read/write
#-----------------------------------------------------------------------------

# write 0x23333 to icache index 0xff bank 1 way 0 data, then read it
icache_data_case1:
  # write inst cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x23333
  csrw cop_data_0, a0
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 1
  csrw cop_bank, a2
  li a2, 0xff # 0xff it is not used by inst
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

  # read inst cache tag using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 1
  csrw cop_bank, a2
  li a2, 0xff # 0xff it is not used by inst
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
  li a2, 0x23333
  bne a1, a2, failure

# write 0x00, 0x01, 0x02, 0x03 ... to icache index 0xff bank 0 way 0, then read
icache_data_case2:
  # write inst cache using cache op
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
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 0xff
  csrw cop_index, a2
  li a2, 0x0
  csrw cop_bank, a2

  # send cache op request
  li a2, COP_WRITE_DATA
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure

  # write inst cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a0, 0x10
  csrw cop_data_0, a0
  li a0, 0x11
  csrw cop_data_1, a0
  li a0, 0x12
  csrw cop_data_2, a0
  li a0, 0x13
  csrw cop_data_3, a0
  li a0, 0x14
  csrw cop_data_4, a0
  li a0, 0x15
  csrw cop_data_5, a0
  li a0, 0x16
  csrw cop_data_6, a0
  li a0, 0x17
  csrw cop_data_7, a0
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 0xff
  csrw cop_index, a2
  li a2, 0x1
  csrw cop_bank, a2

  # send cache op request
  li a2, COP_WRITE_DATA
  csrw cop_op, a2

  # wait for cache op response
  jal wait_until_cop_finish_or_timeout

  # check cache op result
  csrr a1, cop_finish
  li a2, 1
  bne a1, a2, failure

  # read inst cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 0xff
  csrw cop_index, a2
  li a2, 0x0
  csrw cop_bank, a2

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

  # read inst cache using cache op
  # reset cache op csrs
  jal clear_cop_csrs

  # prepare cache op ctrl info
  li a2, COP_ID_ICACHE
  csrw cop_level, a2
  li a2, 0xff
  csrw cop_index, a2
  li a2, 0x1
  csrw cop_bank, a2

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
  li a2, 0x10
  bne a1, a2, failure
  csrr a1, cop_data_1
  li a2, 0x11
  bne a1, a2, failure
  csrr a1, cop_data_2
  li a2, 0x12
  bne a1, a2, failure
  csrr a1, cop_data_3
  li a2, 0x13
  bne a1, a2, failure
  csrr a1, cop_data_4
  li a2, 0x14
  bne a1, a2, failure
  csrr a1, cop_data_5
  li a2, 0x15
  bne a1, a2, failure
  csrr a1, cop_data_6
  li a2, 0x16
  bne a1, a2, failure
  csrr a1, cop_data_7
  li a2, 0x17
  bne a1, a2, failure

#-----------------------------------------------------------------------------
# Test 3: insert icache error
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# test finished
#-----------------------------------------------------------------------------

  j success
  
#-----------------------------------------------------------------------------
# misc
#-----------------------------------------------------------------------------


