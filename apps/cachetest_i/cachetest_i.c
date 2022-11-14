#include <klib.h>
#include "cachetest_i.h"

int main(){
   // printf("111111\n");
    unsigned long long busy_cycles_l1,busy_cycles_l2;
    unsigned long long busy_instrs_l1,busy_instrs_l2;
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
    //uint64_t end_addr = _PERF_TEST_ADDR_BASE + 5*(_TEST_NUM_SIZE);
    uint64_t end_addr_l2 = (_PERF_TEST_ADDR_BASE + _TEST_NUM_SIZE + _PERF_L1_SIZE_BYTE);
    uint64_t end_addr_l1 = (_PERF_TEST_ADDR_BASE + _PERF_L1_SIZE_BYTE - (2*_TEST_NUM_SIZE));
    printf("end_addr_l1 %x  end_addr_l2 %x\n",end_addr_l1,end_addr_l2);
    //printf("222222222222222\n");
    full_cache_init_i(start_addr,end_addr_l1,_PERF_CACHELINE_SIZE_BYTE);
    //printf("finish_full\n");
    //printf("finish_full\n");
    cache_iloop(&busy_instrs_l1,&busy_cycles_l1);

    full_cache_init_i(start_addr,end_addr_l2,_PERF_CACHELINE_SIZE_BYTE);
    cache_iloop(&busy_instrs_l2,&busy_cycles_l2);
  //  busy_cycles =1;
  //  busy_instrs = 1;
    printf("the test of load-add-load hit in l1\n");
    printf("instrs %d cycles %d\n",busy_instrs_l1,busy_cycles_l1);
    printf("the test of load-add-load hit in l2\n");
    printf("instrs %d cycles %d\n",busy_instrs_l2,busy_cycles_l2);


}
