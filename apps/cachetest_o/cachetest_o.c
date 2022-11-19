#include <klib.h>
#include "cachetest_o.h"


int main(){
   // printf("111111\n");
    unsigned long long busy_cycles_l1,busy_cycles_l2,busy_cycles_l3;
    unsigned long long busy_instrs_l1,busy_instrs_l2,busy_instrs_l3;
    
   // unsigned long long busy_cycles_l3;
   // unsigned long long busy_instrs_l3;
    /*unsigned long long busy_cycles_mem;
    unsigned long long busy_instrs_mem;*/
    //uint64_t end_addr = _PERF_TEST_ADDR_BASE + 5*(_PERF_L1_SIZE_BYTE);
    uint64_t end_addr_l2 = (_PERF_TEST_ADDR_BASE + _TEST_NUM_SIZE + (_PERF_L2_SIZE_BYTE/2));
    uint64_t end_addr_l1 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE - (2*_TEST_NUM_SIZE));
    uint64_t end_addr_l3 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE + _PERF_L2_SIZE_BYTE + _PERF_L2_SIZE_BYTE);
   // uint64_t end_addr_mem = (_PERF_TEST_ADDR_BASE + _TEST_NUM_SIZE + _PERF_L1_SIZE_BYTE + _PERF_L2_SIZE_BYTE + _PERF_L3_SIZE_BYTE);
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
     //printf("end_addr_mem %x \n",end_addr_mem -2*_TEST_NUM_SIZE);
   // printf("end_addr_l1 %x end_addr_l2 %x\n",end_addr_l1,end_addr_l2);
    //printf("222222222222222\n");
    full_cache_init(start_addr,end_addr_l1,_PERF_CACHELINE_SIZE_BYTE,1);

    cache_loop(&busy_instrs_l1,&busy_cycles_l1);

    full_cache_init(start_addr,end_addr_l2,_PERF_CACHELINE_SIZE_BYTE,2);
    cache_loop(&busy_instrs_l2,&busy_cycles_l2);
    full_cache_init(start_addr,end_addr_l3,_PERF_CACHELINE_SIZE_BYTE,3);
    cache_loop(&busy_instrs_l3,&busy_cycles_l3);
    //full_cache_init(start_addr,end_addr_mem,_PERF_CACHELINE_SIZE_BYTE);
    //cache_loop(&busy_instrs_mem,&busy_cycles_mem);
    printf("the test of load-load-load hit in l1\n");
    printf("instrs %d cycles %d\n",busy_instrs_l1,busy_cycles_l1);
    printf("the test of load-load-load hit in l2\n");
    printf("instrs %d cycles %d\n",busy_instrs_l2,busy_cycles_l2);
    printf("the test of load-load-load hit in l3\n");
    printf("instrs %d cycles %d\n",busy_instrs_l3,busy_cycles_l3);
    /*printf("the test of load-load-load hit in mem\n");
    printf("instrs %d cycles %d\n",busy_instrs_mem,busy_cycles_mem);*/


}

