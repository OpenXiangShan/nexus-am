#include <klib.h>
#include "mem_test_latency.h"

#define xstr(s) str(s)
#define str(s)  #s

int main(){
    unsigned long long busy_cycles;
    unsigned long long busy_instrs;
    //control
    uint64_t cache_warmup_size_count = atoi(xstr(INST));
    //_PERF_TEST_ADDR_BASE 0x80010000
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
    //init end address, step = 256k,input step size
    //_PERF_L1_SIZE_BYTE (64 * KB) _TEST_NUM_SIZE (32 * KB) _STEP_SIZE (256 *KB)
    uint64_t end_addr = (_PERF_TEST_ADDR_BASE + _PERF_L2_SIZE_BYTE + cache_warmup_size_count *_STEP_SIZE);
    //make cache warmup  put the test address in l1/l2/l3 _PERF_CACHELINE_SIZE_BYTE (8 * BYTE)
    full_cache_warmup_i(start_addr,end_addr,_PERF_CACHELINE_SIZE_BYTE,cache_warmup_size_count);
    //cache latency test
    cache_iloop(&busy_instrs,&busy_cycles);

    printf("the test of load-add-load hit result\n");
    printf("instrs %d cycles %d\n",busy_instrs,busy_cycles);
}