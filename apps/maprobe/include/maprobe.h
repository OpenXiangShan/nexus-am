// basic microarchtectural probe

#ifndef PROBE_H
#define PROBE_H

#include <klib.h>
#include <csr.h>

// perf const
#define BYTE (1)
#define KB (1024*BYTE)
#define MB (1024*KB)
#define GB (1024*MB)

// platform dependent const
// #define _PERF_TEST_ADDR_BASE 0x80400000
#define _PERF_TEST_ADDR_BASE 0x2000400000
#define _PERF_CACHELINE_SIZE_BYTE (64 * BYTE)
#define _PERF_L1_NOALIAS_SIZE_BYTE (32 * KB)
#define _PERF_L1_SIZE_BYTE (128 * KB)
#define _PERF_L2_SIZE_BYTE (512 * KB)
#define _PERF_L3_SIZE_BYTE (2 * MB)
#define _PERF_L1_NUM_WAYS 8
#define _PERF_SET_SIZE_BYTE (_PERF_L1_SIZE_BYTE / _PERF_L1_NUM_WAYS)

// probe const
#define _PERF_BLACKHOLE _PERF_TEST_ADDR_BASE

struct perf
{
    // const to be calibrated at run time
    uint64_t csr_read_cycle; //# of cycles to read mcycle
    uint64_t csr_read_ninst; // # of inst needed to read minstret

    // timer
    uint64_t cycle;
    uint64_t instrcnt;
} perf;

void _perf_start_timer()
{
    perf.cycle = csr_read(CSR_MCYCLE);
    perf.instrcnt = csr_read(CSR_MINSTRET);
}

void _perf_end_timer()
{
    perf.cycle = csr_read(CSR_MCYCLE) - perf.cycle;
    perf.instrcnt = csr_read(CSR_MINSTRET) - perf.instrcnt;
}

void _perf_print_timer()
{
    printf("cycle %d inst %d ipc %lf\n", perf.cycle, perf.instrcnt, (float)perf.instrcnt/perf.cycle);
}

void _perf_calibrate()
{
    // csr read delay
    uint64_t cycle_1 = csr_read(CSR_MCYCLE);
    uint64_t cycle_2 = csr_read(CSR_MCYCLE);
    perf.csr_read_cycle = cycle_2-cycle_1;
    printf("perf_calibrate: csr_read_cycle %d\n", perf.csr_read_cycle);

    // csr read inst cost
    uint64_t inst_1 = csr_read(CSR_MINSTRET);
    uint64_t inst_2 = csr_read(CSR_MINSTRET);
    perf.csr_read_ninst = inst_2-inst_1;
    printf("perf_calibrate: csr_read_ninst %d\n", perf.csr_read_ninst);
}

void _perf_blackhole(uint64_t value)
{
    *(uint64_t*) _PERF_BLACKHOLE = value;
}

uint64_t setup_latency_test_linklist(uint64_t base_addr, uint64_t end_addr, uint64_t step)
{
    uint64_t num_valid_node = 0;
    assert(step % 8 == 0);
    assert(step >= 8);
    for (uint64_t cur_addr = base_addr; cur_addr < end_addr;) {
        uint64_t next_addr = cur_addr + step;
        *((uint64_t*)cur_addr) = next_addr;
        cur_addr = next_addr;
        num_valid_node++;
    }
    return num_valid_node;
}

uint64_t read_latency_test_linklist(uint64_t base_addr, uint64_t num_valid_node)
{
    uint64_t cur_addr = base_addr;
    for (int i = 0; i < num_valid_node; i++) {
        cur_addr = (*(uint64_t*)cur_addr);
    }
    return cur_addr;
}

void warmup(uint64_t base_addr, uint64_t end_addr)
{
    setup_latency_test_linklist(base_addr, end_addr, _PERF_CACHELINE_SIZE_BYTE);
}

void test_latency(uint64_t size, int iter)
{
    volatile uint64_t result = 0; // make sure compiler will not opt read_latency_test_linklist
    printf("range 0x%xB (%d iters) latency test\n", size, iter);
    _perf_start_timer();
    uint64_t nnode = setup_latency_test_linklist(_PERF_TEST_ADDR_BASE, _PERF_TEST_ADDR_BASE + size, _PERF_CACHELINE_SIZE_BYTE);
    _perf_end_timer();
    uint64_t total_node = nnode * iter;
    // _perf_print_timer();

    _perf_start_timer();
    for (int i = 0; i < iter; i++) {
        result += read_latency_test_linklist(_PERF_TEST_ADDR_BASE, nnode);
    }
    _perf_end_timer();
    // _perf_print_timer();
    printf("range 0x%xB (%d intrs) read latency %f (%d samples)\n", size, iter, (float)perf.cycle / total_node, total_node);

    _perf_blackhole(result);
}

void test_mem_throughput(uint64_t iter)
{
    uint64_t remain = iter;
    uint64_t result = 0;
    uint64_t access_addr = _PERF_TEST_ADDR_BASE;
    _perf_start_timer();
    while (remain--) {
        result += *(uint64_t*) access_addr;
        access_addr += _PERF_CACHELINE_SIZE_BYTE;
    }
    _perf_end_timer();
    *(uint64_t*) _PERF_BLACKHOLE = result;
    printf("mem band width %f B/cycle (%d samples)\n", (float)iter * _PERF_CACHELINE_SIZE_BYTE / perf.cycle, iter);
}

void test_mem_throughput_same_set(uint64_t iter)
{
    uint64_t remain = iter;
    uint64_t result = 0;
    uint64_t access_addr = _PERF_TEST_ADDR_BASE;
    _perf_start_timer();
    while (remain--) {
        result += *(uint64_t*) access_addr;
        access_addr += _PERF_SET_SIZE_BYTE;
    }
    _perf_end_timer();
    *(uint64_t*) _PERF_BLACKHOLE = result;
    printf("mem band width %f B/cycle (%d samples)\n", (float)iter * _PERF_CACHELINE_SIZE_BYTE / perf.cycle, iter);
}

#endif