#include <klib.h>
#include "cachetest_o.h"

int main(){
   // printf("111111\n");
    unsigned long long busy_cycles;
    unsigned long long busy_instrs;
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
    uint64_t end_addr = _PERF_TEST_ADDR_BASE + 2*(_PERF_L1_SIZE_BYTE);
    //printf("222222222222222\n");
    full_cache_init(start_addr,end_addr,_PERF_CACHELINE_SIZE_BYTE);
    //printf("finish_full\n");
    //printf("finish_full\n");
    cache_loop(&busy_instrs,&busy_cycles);
  //  busy_cycles =1;
  //  busy_instrs = 1;
    printf("the test of load-load-load hit in l2\n");
    printf("instrs %d cycles %d\n",busy_instrs,busy_cycles);


}
