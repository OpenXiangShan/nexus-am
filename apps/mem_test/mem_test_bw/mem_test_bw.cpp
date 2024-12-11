//#ifndef PROBE_H
//#define PROBE_H

#include <klib.h>
#include <csr.h>
#include <stdint.h>
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
#define _PERF_STRIDE_AVOID_BC (8 * BYTE)
#define _PERF_STRIDE_AVOID_BC_NEWLINE (_PERF_CACHELINE_SIZE_BYTE + _PERF_STRIDE_AVOID_BC)
#define _PERF_PAGE_SIZE_BYTE (4 * KB)
#define _PERF_L1_NOALIAS_SIZE_BYTE (16 * KB)
#define _PERF_L1_SIZE_BYTE (64 * KB)
#define _PERF_L1_SIZE_AVOID_BC_BYTE (512 * _PERF_STRIDE_AVOID_BC_NEWLINE)
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

template <unsigned int Stride, unsigned int N>
static inline void accessNElements(uint64_t addr) {
    if constexpr (N > 0) {
        accessNElements<Stride, N - 1>(addr);
        uint64_t __attribute__((unused)) useless;
        asm volatile("ld %0, %1(%2)" : "=r"(useless) : "i"(Stride*(N-1)), "r"(addr) : "memory");
    }
}

template <unsigned int Stride, unsigned int Unroll>
void access_region_with_step_by_xtimes(uint64_t times, uint64_t start_addr, uint64_t region_size) {
    uint64_t remain = times;
    assert(region_size % (Unroll * Stride) == 0);
    assert(remain % (region_size / (4 * Stride)) == 0);
    uint64_t access_addr = start_addr;

    if (region_size / Stride >= times) {
        while (remain -= Unroll) {
            accessNElements<Stride, Unroll>(access_addr);
            access_addr += Stride * Unroll;
        }

    } else {
        while (remain > 0) {
            int64_t inner_remain = region_size / Stride;
            remain -= inner_remain;
            while (inner_remain -= Unroll) {
                accessNElements<Stride, Unroll>(access_addr);
                access_addr += Stride * Unroll;
            }
            inner_remain -= region_size / Stride;
        }
    }
}

template <unsigned int Stride>
void legacy_test_mem_bandwidth(const char *cache_name, uint64_t iter, uint64_t footprint_cachelines, uint64_t footprint_per_sample)
{
    uint64_t cycle_1;
    uint64_t cycle_2;
    /*uint64_t inst_1;*/
    /*uint64_t inst_2;*/
    uint64_t cycle;

    /*printf("%s bandwidth test\n", cache_name);*/
    // warmup
    access_region_with_step_by_xtimes<Stride, 8>(
        iter, _PERF_TEST_ADDR_BASE,  footprint_cachelines * Stride);
    // test
    cycle_1 = csr_read(CSR_MCYCLE);
    /*inst_1  = csr_read(CSR_MINSTRET); */
    access_region_with_step_by_xtimes<Stride, 8>(
        iter, _PERF_TEST_ADDR_BASE, footprint_cachelines * Stride);
    cycle_2 = csr_read(CSR_MCYCLE);
    /*inst_2  = csr_read(CSR_MINSTRET);*/
    cycle = cycle_2 - cycle_1;
    /*inst  = inst_2 - inst_1;*/
    /*printf("%s bandwidth=%fBytes/cycle (%d samples) inst %ld cycle %ld, checksum=%lx\n",*/
    printf("%s bandwidth=%.2f Bytes/cycle (%d samples, footprint of %d cachelins) with Stride = %u\n",
           cache_name, (float)iter * footprint_per_sample / cycle, iter, footprint_cachelines, Stride);
}

int main(){
    printf("start ddr test\n");
    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>("L1", 2048, 128, 64);
    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC>("L1", 2048, 128, 64);
    legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>("L1", 2048, 128, 64);

    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>("L1",  2048, 256, 64);
    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC>("L1", 2048, 256, 64);
    legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>("L1", 2048, 256, 64);

    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>("L2", 2048, 1024, 64);
    legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>("L2", 2048, 1024, 64);

    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>("L2", 2048, 2048, 64);
    legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>("L2", 2048, 2048, 64);

    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>("L2", 4096, 4096, 64);
    legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>("L2", 4096, 4096, 64);

    legacy_test_mem_bandwidth<_PERF_STRIDE_AVOID_BC_NEWLINE>("L2", 8*1024, 8*1024, 64);
    legacy_test_mem_bandwidth<_PERF_CACHELINE_SIZE_BYTE>("L2", 8*1024, 8*1024, 64);


    // legacy_test_mem_throughput_same_set(500);
    return 0;
}
