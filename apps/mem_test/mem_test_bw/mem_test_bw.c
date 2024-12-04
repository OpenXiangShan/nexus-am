//#ifndef PROBE_H
//#define PROBE_H

#include <klib.h>
#include <csr.h>
//#include "bitutils.h"
//#include "resultmat.h"

// config
// #define PERF_SIM // probe run in simulatior, diaable perf counters

// perf const
#define BYTE (1)
#define KB (1024*BYTE)
#define MB (1024*KB)
#define GB (1024*MB)

// platform dependent const
#ifndef _PERF_TEST_ADDR_BASE
#define _PERF_TEST_ADDR_BASE 0x80400000
#define _PERF_TEST_ADDR_BASE1 0x80100000
// #define _PERF_TEST_ADDR_BASE 0x2000400000
#endif
#define _PERF_CACHELINE_SIZE_BYTE (64 * BYTE)
#define _PERF_CACHELINE_SIZE_BYTE_STEP (8 * BYTE)
#define _PERF_PAGE_SIZE_BYTE (4 * KB)
#define _PERF_L1_NOALIAS_SIZE_BYTE (16 * KB)
#define _PERF_L1_SIZE_BYTE (64 * KB)
#define _PERF_L2_SIZE_BYTE (1 * MB)
#define _PERF_L3_SIZE_BYTE (6 * MB)
#define _PERF_MEM_SIZE_BYTE (1024 * MB)
#define _PERF_L1_NUM_WAYS 4
#define _PERF_L1_NUM_SETS 256
#define _PERF_L2_NUM_WAYS 8
#define _PERF_L2_NUM_SLICES 4
#define _PERF_L2_NUM_SETS 512

#define _PERF_ADDR_STRIDE_L1_SAME_BANK _PERF_CACHELINE_SIZE_BYTE
#define _PERF_ADDR_STRIDE_L1_SAME_SET (_PERF_L1_NUM_SETS * _PERF_CACHELINE_SIZE_BYTE)
#define _PERF_ADDR_STRIDE_L2_SAME_SLICE (_PERF_L2_NUM_SLICES * _PERF_CACHELINE_SIZE_BYTE)
#define _PERF_ADDR_STRIDE_L1_SAME_SET (_PERF_L1_NUM_SETS * _PERF_CACHELINE_SIZE_BYTE)
#define _PERF_ADDR_STRIDE_L2_SAME_SET (_PERF_L2_NUM_SLICES * _PERF_L2_NUM_SETS * _PERF_CACHELINE_SIZE_BYTE)
#define _PERF_ADDR_STRIDE_NEXT_PAGE (_PERF_PAGE_SIZE_BYTE)

// probe const
#define _PERF_BLACKHOLE _PERF_TEST_ADDR_BASE

void legacy_test_mem_throughput(uint64_t iter)
{
    uint64_t remain = iter;
    uint64_t result = 0;
    uint64_t access_addr = _PERF_TEST_ADDR_BASE;
    uint64_t cycle_1;
    uint64_t cycle_2;
    uint64_t inst_1;
    uint64_t inst_2;
    uint64_t cycle;
    uint64_t inst;

    cycle_1 = csr_read(CSR_MCYCLE);
    inst_1  = csr_read(CSR_MINSTRET); 
    
    // Unroll this loop by four times to avoid fragmenting instruction supply
#define STEP 4
    assert(iter %  STEP == 0);
#pragma GCC unroll 4

    while (remain -= 1) {
        uint64_t * restrict ptr = (uint64_t *)access_addr;
        result += *ptr;
        access_addr += _PERF_CACHELINE_SIZE_BYTE;
    }
    //_perf_end_timer();
    cycle_2 = csr_read(CSR_MCYCLE);
    inst_2  = csr_read(CSR_MINSTRET);

    cycle = cycle_2 - cycle_1;
    inst  = inst_2 - inst_1;
    *(uint64_t*) _PERF_BLACKHOLE = result;
    printf("mem band width %f B/cycle (%d samples) inst %ld, checksum=%lx\n", (float)iter * _PERF_CACHELINE_SIZE_BYTE / cycle, iter, inst, result);
}

void legacy_test_mem_throughput_same_set(uint64_t iter)
{
    uint64_t remain = iter;
    uint64_t result = 0;
    uint64_t access_addr = _PERF_TEST_ADDR_BASE1;
    uint64_t cycle_1;
    uint64_t cycle_2;
    uint64_t inst_1;
    uint64_t inst_2;
    uint64_t cycle;
    uint64_t inst;
    //_perf_start_timer();
    cycle_1 = csr_read(CSR_MCYCLE);
    inst_1  = csr_read(CSR_MINSTRET); 
    while (remain--) {
        result += *(uint64_t*) access_addr;
        access_addr += _PERF_CACHELINE_SIZE_BYTE;
    }
    cycle_2 = csr_read(CSR_MCYCLE);
    inst_2  = csr_read(CSR_MINSTRET);
    cycle = cycle_2 - cycle_1;
    inst  = inst_2 - inst_1;
    //*(uint64_t*) _PERF_BLACKHOLE = result;
    //_perf_end_timer();
    *(uint64_t*) _PERF_BLACKHOLE = result;
    printf("mem band width %f B/cycle (%d samples) inst %ld\n", (float)iter * _PERF_CACHELINE_SIZE_BYTE / cycle, iter, inst);
}

int main(){
    printf("start ddr test\n");
    legacy_test_mem_throughput(2000);
    // legacy_test_mem_throughput_same_set(500);

}
