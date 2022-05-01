#include <amtest.h>

#define cop_op_idx 0
#define cop_finish_idx 1
#define cop_level_idx 2
#define cop_way_idx 3
#define cop_index_idx 4
#define cop_bank_idx 5
#define cop_tag_ecc_idx 6
#define cop_tag_bits_idx 7
#define cop_tag_data_idx 8
#define cop_tag_data_h_idx 9
#define cop_ecc_width_idx 10
#define cop_data_ecc_idx 11
#define cop_data_0_idx 0
#define cop_data_1_idx 1
#define cop_data_2_idx 2
#define cop_data_3_idx 3
#define cop_data_4_idx 4
#define cop_data_5_idx 5
#define cop_data_6_idx 6
#define cop_data_7_idx 7

#define COP_ID_ICACHE 0
#define COP_ID_DCACHE 1

#define COP_READ_TAG_ECC 0
#define COP_READ_DATA_ECC 1
#define COP_READ_TAG 2
#define COP_READ_DATA 3
#define COP_WRITE_TAG_ECC 4
#define COP_WRITE_DATA_ECC 5
#define COP_WRITE_TAG 6
#define COP_WRITE_DATA 7
#define COP_FLUSH_BLOCK 8

void success() {
    printf("Cache op test passed.\n");
    asm("li a0, 0\n");
    asm(".word 0x0000006b\n");
}

void failure() {
    printf("Cache op test failed\n");
    asm("li a0, 1\n");
    asm(".word 0x0000006b\n");
}

void clear_cop_csrs() {
    _l1cache_op_write(cop_op_idx, 0);
    _l1cache_op_write(cop_finish_idx, 0);
    _l1cache_op_write(cop_level_idx, 0);
    _l1cache_op_write(cop_way_idx, 0);
    _l1cache_op_write(cop_index_idx, 0);
    _l1cache_op_write(cop_bank_idx, 0);
    _l1cache_op_write(cop_tag_ecc_idx, 0);
    _l1cache_op_write(cop_tag_bits_idx, 0);
    _l1cache_op_write(cop_tag_data_idx, 0);
    _l1cache_op_write(cop_tag_data_h_idx, 0);
    _l1cache_op_write(cop_ecc_width_idx, 0);
    _l1cache_op_write(cop_data_ecc_idx, 0);
    _l1cache_data_write(cop_data_0_idx, 0);
    _l1cache_data_write(cop_data_1_idx, 0);
    _l1cache_data_write(cop_data_2_idx, 0);
    _l1cache_data_write(cop_data_3_idx, 0);
    _l1cache_data_write(cop_data_4_idx, 0);
    _l1cache_data_write(cop_data_5_idx, 0);
    _l1cache_data_write(cop_data_6_idx, 0);
    _l1cache_data_write(cop_data_7_idx, 0);
}

void wait_until_cop_finish_or_timeout() {
    int t1 = 0x0;
    int t0 = 0x1000;
    uintptr_t csrread = 0;
    while(t1 < t0) {
        csrread = _l1cache_op_read(cop_finish_idx);
        if(csrread != 0) {
            return;
        }
        t1 += 1;
    }
}

void cache_test() {
    printf("Cache op test: difftest should be disabled\n");
    printf("Hint: use --no-diff to disable difftest\n");

    //-----------------------------------------------------------------------------
    // Test 1: cache op ctrl csr read / write
    //-----------------------------------------------------------------------------

    // basic csr read/write
    _l1cache_op_write(cop_finish_idx, 0);
    uintptr_t csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 0) {
        failure();
        return;
    }

    _l1cache_op_write(cop_level_idx, 0x12345678);
    csrread = _l1cache_op_read(cop_level_idx);
    if(csrread != 0x12345678) {
        failure();
        return;
    }

    _l1cache_data_write(cop_data_0_idx, 0x12345678);
    csrread = _l1cache_data_read(cop_data_0_idx);
    if(csrread != 0x12345678) {
        failure();
        return;
    }

    // basic dcache data write op
    clear_cop_csrs();       //reset cache op csrs

    _l1cache_data_write(cop_data_0_idx, 0x12345678);
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_op_idx, COP_WRITE_DATA);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    //-----------------------------------------------------------------------------
    // Test 2: basic cache op flow
    //-----------------------------------------------------------------------------

    // dcache data write/read op: loop test

    // write 0x12345678, 0x0, 0x0 ... to dcache index 0 way 0
    
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_data_write(cop_data_0_idx, 0x12345678);
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_WRITE_DATA);
 
    wait_until_cop_finish_or_timeout();        //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }

    // read data cache using cache op
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_op_idx, COP_READ_DATA);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }

    csrread = _l1cache_data_read(cop_data_0_idx);
    if(csrread != 0x12345678) {
        failure();
        return;
    }

    //-----------------------------------------------------------------------------
    // Test 3: dcache data read/write
    //-----------------------------------------------------------------------------

    // write 0x0, 0x1, 0x2, 0x3 ... to dcache index 1 way 1

    // write data cache using cache op
    
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_data_write(cop_data_0_idx, 0x0);
    _l1cache_data_write(cop_data_1_idx, 0x1);
    _l1cache_data_write(cop_data_2_idx, 0x2);
    _l1cache_data_write(cop_data_3_idx, 0x3);
    _l1cache_data_write(cop_data_4_idx, 0x4);
    _l1cache_data_write(cop_data_5_idx, 0x5);
    _l1cache_data_write(cop_data_6_idx, 0x6);
    _l1cache_data_write(cop_data_7_idx, 0x7);

    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_WRITE_DATA);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }


    // read data cache using cache op
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_READ_DATA);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }

    csrread = _l1cache_data_read(cop_data_0_idx);
    if(csrread != 0) {
        failure();
        return;
    }
    csrread = _l1cache_data_read(cop_data_1_idx);
    if(csrread != 1) {
        failure();
        return;
    }
    csrread = _l1cache_data_read(cop_data_2_idx);
    if(csrread != 2) {
        failure();
        return;
    }
    csrread = _l1cache_data_read(cop_data_3_idx);
    if(csrread != 3) {
        failure();
        return;
    }
    csrread = _l1cache_data_read(cop_data_4_idx);
    if(csrread != 4) {
        failure();
        return;
    }
    csrread = _l1cache_data_read(cop_data_5_idx);
    if(csrread != 5) {
        failure();
        return;
    }
    csrread = _l1cache_data_read(cop_data_6_idx);
    if(csrread != 6) {
        failure();
        return;
    }
    csrread = _l1cache_data_read(cop_data_7_idx);
    if(csrread != 7) {
        failure();
        return;
    }

    //-----------------------------------------------------------------------------
    // Test 4: dcache tag read/write
    //-----------------------------------------------------------------------------

    // write 0x233 to dcache index 1 way 1 tag, then read it
    // write data cache using cache op
    
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_tag_data_idx, 0x233);
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);
 
    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_WRITE_TAG);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }

    // read data cache tag using cache op
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_READ_TAG);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }
    csrread = _l1cache_op_read(cop_tag_data_idx);
    if(csrread != 0x233) {
        failure();
        return;
    }

    //-----------------------------------------------------------------------------
    // Test 5: dcache ecc read/write
    //-----------------------------------------------------------------------------

    // write 0x3 to dcache index 1 way 1 bank 1 data ecc, then read it
    
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_data_ecc_idx, 0x3);
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_WRITE_DATA_ECC);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }

    // read data cache tag using cache op
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_READ_DATA_ECC);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }
    csrread = _l1cache_op_read(cop_data_ecc_idx);
    if(csrread != 0x3) {
        failure();
        return;
    }

    // write 0x3 to dcache index 1 way 1 bank 1 tag ecc, then read it

    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_tag_ecc_idx, 0x3);
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_WRITE_TAG_ECC);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }

    // read data cache tag using cache op
    clear_cop_csrs();       //reset cache op csrs

    // prepare cache op ctrl info
    _l1cache_op_write(cop_level_idx, COP_ID_DCACHE);
    _l1cache_op_write(cop_way_idx, 0x1);
    _l1cache_op_write(cop_index_idx, 0x1);

    // send cache op request
    _l1cache_op_write(cop_op_idx, COP_READ_TAG_ECC);

    wait_until_cop_finish_or_timeout();       //wait for cache op response

    // check cache op result
    csrread = _l1cache_op_read(cop_finish_idx);
    if(csrread != 1) {
        failure();
        return;
    }
    csrread = _l1cache_op_read(cop_tag_ecc_idx);
    if(csrread != 0x3) {
        failure();
        return;
    }

    success();
}