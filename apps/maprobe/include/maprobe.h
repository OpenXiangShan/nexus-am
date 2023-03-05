// basic microarchtectural probe

#ifndef PROBE_H
#define PROBE_H

#include <klib.h>
#include <csr.h>
#include "bitutils.h"

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
#define _PERF_L1_NOALIAS_SIZE_BYTE (32 * KB)
#define _PERF_L1_SIZE_BYTE (64 * KB)
#define _PERF_L2_SIZE_BYTE (512 * KB)
#define _PERF_L3_SIZE_BYTE (2 * MB)
#define _PERF_L1_NUM_WAYS 4
#define _PERF_SET_SIZE_BYTE (_PERF_L1_SIZE_BYTE / _PERF_L1_NUM_WAYS)

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

// common perf tools
extern void _perf_start_timer();
extern void _perf_end_timer();
extern void _perf_print_timer();
extern void _perf_calibrate();
extern void _perf_blackhole(uint64_t value);

// latency test
extern uint64_t setup_latency_test_linklist(uint64_t base_addr, uint64_t end_addr, uint64_t step);
extern uint64_t read_latency_test_linklist(uint64_t base_addr, uint64_t num_valid_node);
extern void latency_test_warmup(uint64_t base_addr, uint64_t end_addr);
extern void test_latency(uint64_t size, int iter);
extern void test_mem_throughput(uint64_t iter);
extern void test_mem_throughput_same_set(uint64_t iter);

#endif