#include <xs.h>
#include <cache.h>
#include <csr.h>
#include <klib.h>

// these functions are interface for cache
// uintptr_t _l1cache_op_read(int op_idx): return value in the cache cop csr
// uintptr_t _l1cache_data_read(int data_idx): return data in the cache data csr
// void _l1cache_op_write(int op_idx, uintptr_t val): set value in the cache cop csr
// void _l1cache_data_write(int data_idx, uintptr_t val): set data in the cache data csr
// when testing, can use these interface in nexus-am/tests/amtest/src/tests/cache.c

// void _l3cache_tag(int tag): set value of tag in l3chache
// void _l3cache_set(int set): set value of tag in l3chache
// void _l3cache_cmd(int cmd): set value of tag in l3chache
// when testing, can use these interface in nexus-am/tests/dualcoretest/tests/huancunop.c


uintptr_t _l1cache_op_read(int op_idx) {
    uintptr_t op;
    switch(op_idx){
        case 0: asm volatile("csrr %0, 0x5C5" : "=r"(op)); break;    //cop_op
        case 1: asm volatile("csrr %0, 0x5C6" : "=r"(op)); break;    //cop_finish
        case 2: asm volatile("csrr %0, 0x5C7" : "=r"(op)); break;    //cop_level
        case 3: asm volatile("csrr %0, 0x5C8" : "=r"(op)); break;    //cop_way
        case 4: asm volatile("csrr %0, 0x5C9" : "=r"(op)); break;    //cop_index
        case 5: asm volatile("csrr %0, 0x5CA" : "=r"(op)); break;    //cop_bank
        case 6: asm volatile("csrr %0, 0x5CB" : "=r"(op)); break;    //cop_tag_ecc
        case 7: asm volatile("csrr %0, 0x5CC" : "=r"(op)); break;    //cop_tag_bits
        case 8: asm volatile("csrr %0, 0x5CD" : "=r"(op)); break;    //cop_tag_data
        case 9: asm volatile("csrr %0, 0x5CE" : "=r"(op)); break;    //cop_tag_data_h
        case 10: asm volatile("csrr %0, 0x5CF" : "=r"(op)); break;   //cop_ecc_width
        case 11: asm volatile("csrr %0, 0x5D0" : "=r"(op)); break;   //cop_data_ecc
        default: op = 0; break;
    }
    return op;
}

uintptr_t _l1cache_data_read(int data_idx) {
    uintptr_t data;
    switch(data_idx){
        case 0: asm volatile("csrr %0, 0x5D1" : "=r"(data)); break;    //cop_data_0
        case 1: asm volatile("csrr %0, 0x5D2" : "=r"(data)); break;    //cop_data_1
        case 2: asm volatile("csrr %0, 0x5D3" : "=r"(data)); break;    //cop_data_2
        case 3: asm volatile("csrr %0, 0x5D4" : "=r"(data)); break;    //cop_data_3
        case 4: asm volatile("csrr %0, 0x5D5" : "=r"(data)); break;    //cop_data_4
        case 5: asm volatile("csrr %0, 0x5D6" : "=r"(data)); break;    //cop_data_5
        case 6: asm volatile("csrr %0, 0x5D7" : "=r"(data)); break;    //cop_data_6
        case 7: asm volatile("csrr %0, 0x5D8" : "=r"(data)); break;    //cop_data_7
        default: data = 0; break;
    }
    return data;
}

void _l1cache_op_write(int op_idx, uintptr_t val) {
    switch(op_idx){
        case 0: asm volatile("csrw 0x5C5, %0" : : "r"(val)); break;    //cop_op
        case 1: asm volatile("csrw 0x5C6, %0" : : "r"(val)); break;    //cop_finish
        case 2: asm volatile("csrw 0x5C7, %0" : : "r"(val)); break;    //cop_level
        case 3: asm volatile("csrw 0x5C8, %0" : : "r"(val)); break;    //cop_way
        case 4: asm volatile("csrw 0x5C9, %0" : : "r"(val)); break;    //cop_index
        case 5: asm volatile("csrw 0x5CA, %0" : : "r"(val)); break;    //cop_bank
        case 6: asm volatile("csrw 0x5CB, %0" : : "r"(val)); break;    //cop_tag_ecc
        case 7: asm volatile("csrw 0x5CC, %0" : : "r"(val)); break;    //cop_tag_bits
        case 8: asm volatile("csrw 0x5CD, %0" : : "r"(val)); break;    //cop_tag_data
        case 9: asm volatile("csrw 0x5CE, %0" : : "r"(val)); break;    //cop_tag_data_h
        case 10: asm volatile("csrw 0x5CF, %0" : : "r"(val)); break;   //cop_ecc_width
        case 11: asm volatile("csrw 0x5D0, %0" : : "r"(val)); break;   //cop_data_ecc
        default: break;
    }
}

void _l1cache_data_write(int data_idx, uintptr_t val) {
    switch(data_idx){
        case 0: asm volatile("csrw 0x5D1, %0" : : "r"(val)); break;    //cop_data_0
        case 1: asm volatile("csrw 0x5D2, %0" : : "r"(val)); break;    //cop_data_1
        case 2: asm volatile("csrw 0x5D3, %0" : : "r"(val)); break;    //cop_data_2
        case 3: asm volatile("csrw 0x5D4, %0" : : "r"(val)); break;    //cop_data_3
        case 4: asm volatile("csrw 0x5D5, %0" : : "r"(val)); break;    //cop_data_4
        case 5: asm volatile("csrw 0x5D6, %0" : : "r"(val)); break;    //cop_data_5
        case 6: asm volatile("csrw 0x5D7, %0" : : "r"(val)); break;    //cop_data_6
        case 7: asm volatile("csrw 0x5D8, %0" : : "r"(val)); break;    //cop_data_7
        default: break;
    }
}

void _l3cache_tag(int tag) {
    *(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = tag;
}

void _l3cache_set(int set) {
    *(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = set;
}

void _l3cache_cmd(int cmd) {
    (*(uint64_t*)CACHE_CMD_BASE) = cmd;
}