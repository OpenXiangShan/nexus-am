// basic microarchtectural probe

#ifndef PROBE_H
#define PROBE_H

#include <klib.h>
#include <csr.h>
#include "bitutils.h"
#include "resultmat.h"

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
// #define _PERF_TEST_ADDR_BASE 0x2000400000
#endif
#define _PERF_CACHELINE_SIZE_BYTE (64 * BYTE)
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

struct perf
{
    // const to be calibrated at run time
    uint64_t csr_read_cycle; // # of cycles to read mcycle
    uint64_t csr_read_ninst; // # of inst needed to read minstret

    // timer
    uint64_t cycle;
    uint64_t instrcnt;
};
extern struct perf perf;

extern uint64_t _perf_g_total_samples;

// common perf tools
extern void _perf_start_timer();
extern void _perf_end_timer();
extern void _perf_print_timer();
extern void _perf_calibrate();
extern void _perf_blackhole(uint64_t value);

// latency test
extern uint64_t setup_pointer_tracing_linklist(uint64_t base_addr, uint64_t end_addr, uint64_t step);
extern uint64_t read_pointer_tracing_linklist(uint64_t base_addr, uint64_t num_valid_node);
extern void latency_test_warmup(uint64_t base_addr, uint64_t end_addr);
extern float test_pointer_tracing_latency(uint64_t size, int step, int iter, int to_csv);
extern float test_linear_access_latency(uint64_t size, uint64_t step, int iter, int to_csv);
extern float test_linear_access_latency_simple(uint64_t size, uint64_t step, int iter, int to_csv);
extern float test_linear_access_latency_batch8(uint64_t size, uint64_t step, int iter, int to_csv);
extern float test_random_access_latency(uint64_t num_access, uint64_t test_range, uint64_t test_align, int pregen_addr, int iter, int to_csv);
extern float test_same_address_load_latency(int iter, int to_csv);
extern float test_read_after_write_latency(int iter, int to_csv);
extern float test_linear_write_latency(uint64_t size, uint64_t step, int iter, int to_csv);


// bandwidth test
extern float test_l1_load_bandwidth(uint64_t size, int iter, int to_csv);
extern float test_l1_store_bandwidth(uint64_t size, int iter, int to_csv);
extern float test_l1_store_wcb_bandwidth(uint64_t size, int iter, int to_csv);

// key parameter matrix generate
void generate_linear_access_latency_matrix(uint64_t step);
void generate_pointer_tracing_latency_matrix(uint64_t step);
void generate_random_access_latency_matrix();
void generate_replacement_test_matrix();

// legacy test
extern void legacy_test_mem_throughput(uint64_t iter);
extern void legacy_test_mem_throughput_same_set(uint64_t iter);

#endif