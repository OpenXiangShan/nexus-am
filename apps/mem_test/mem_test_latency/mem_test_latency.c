#include <klib.h>
#include "mem_test_latency.h"

#define xstr(s) str(s)
#define str(s)  #s

void flush_cache_efficient(unsigned size_in_kb, uint64_t start_addr) {
  unsigned bytes = 1024 * size_in_kb;
  uint64_t blackhole = 0;
  for (unsigned i = 0; i < bytes; i += 64) {
    blackhole += *(uint64_t *)(start_addr + i);;
  }
  printf("Flushed %u KB cache, blackhole = %lx\n", size_in_kb, blackhole);
}

int main() {
    unsigned long long busy_cycles;
    unsigned long long busy_instrs;
    //control
    //_PERF_TEST_ADDR_BASE 0x80010000
    uint64_t start_addr = _PERF_TEST_ADDR_BASE;
    //init end address, step = 256k,input step size
    //_PERF_L1_SIZE_BYTE (64 * KB) _TEST_NUM_SIZE (32 * KB) _STEP_SIZE (256 *KB)

    unsigned footprint_kb = 256 * atoi(xstr(INST));
    int block_count = 4000;
    uint64_t flush_start = start_addr + 0x40000000;
    printf("Intializing linked-list of %i nodes in range 0x%lx-0x%lx with seed %#x.\n"
          "Then flushing cache of %i KB starting from %#lx\n",
          block_count, start_addr, flush_start, 0xdeadbeaf, footprint_kb, flush_start);
    pseudo_random_warmup(start_addr, 0xdeadbeaf /*non-zero*/, block_count, 0x40000000);
    //flush cache
    flush_cache_efficient(footprint_kb, flush_start);
    //cache latency test
    cache_iloop(&busy_instrs, &busy_cycles);
    printf("the test of load-add-load hit result\n");
    printf("instrs %d cycles %d\n",busy_instrs,busy_cycles);
}
