#include <klib.h>
#include "mem_test_latency.h"

#define xstr(s) str(s)
#define str(s)  #s

int main(){
    unsigned long long busy_cycles;
    unsigned long long busy_instrs;
    //control
    //_PERF_TEST_ADDR_BASE 0x80010000
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
    //init end address, step = 256k,input step size
    //_PERF_L1_SIZE_BYTE (64 * KB) _TEST_NUM_SIZE (32 * KB) _STEP_SIZE (256 *KB)

    int block_count = (atoi(xstr(INST)) * 256 * 1024)/64;
    printf("Intializing %i KB caches in range 0x%lx-0x%lx with seed %#x\n",
          block_count*64/1024, start_addr, start_addr + 0x40000000, 0xdeadbeaf);
    pseudo_random_warmup(start_addr, 0xdeadbeaf /*non-zero*/, block_count, 0x40000000);
    //cache latency test
    cache_iloop(&busy_instrs,&busy_cycles);

    printf("the test of load-add-load hit result\n");
    printf("instrs %d cycles %d\n",busy_instrs,busy_cycles);
}
