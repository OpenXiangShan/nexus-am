#include <klib.h>
#include "cachetest_i.h"

int main(){
    unsigned long long busy_cycles_l1,busy_cycles_l2,busy_cycles_l3;
    unsigned long long busy_instrs_l1,busy_instrs_l2,busy_instrs_l3;
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;

    uint64_t end_addr_l2 = (_PERF_TEST_ADDR_BASE + _TEST_NUM_SIZE + (_PERF_L2_SIZE_BYTE/2));
    uint64_t end_addr_l1 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE - (2*_TEST_NUM_SIZE));
    uint64_t end_addr_l3 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE + _PERF_L2_SIZE_BYTE + _PERF_L2_SIZE_BYTE);

    full_cache_init_i(start_addr,end_addr_l1,_PERF_CACHELINE_SIZE_BYTE,1);
    cache_iloop(&busy_instrs_l1,&busy_cycles_l1);

    full_cache_init_i(start_addr,end_addr_l2,_PERF_CACHELINE_SIZE_BYTE,2);
    cache_iloop(&busy_instrs_l2,&busy_cycles_l2);

    full_cache_init_i(start_addr,end_addr_l3,_PERF_CACHELINE_SIZE_BYTE,3);
    cache_iloop(&busy_instrs_l3,&busy_cycles_l3);


    printf("the test of load-add-load hit in l1\n");
    printf("instrs %d cycles %d\n",busy_instrs_l1,busy_cycles_l1);
    printf("the test of load-add-load hit in l2\n");
    printf("instrs %d cycles %d\n",busy_instrs_l2,busy_cycles_l2);
    printf("the test of load-add-load hit in l3\n");
    printf("instrs %d cycles %d\n",busy_instrs_l3,busy_cycles_l3);
    //printf("the test of load-add-load hit in mem\n");
    //printf("instrs %d cycles %d\n",busy_instrs_mem,busy_cycles_mem);
    printf("the test is finish\n");


}
