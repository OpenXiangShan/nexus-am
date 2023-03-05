#include "maprobe.h"

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

void latency_test_warmup(uint64_t base_addr, uint64_t end_addr)
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
    printf("range 0x%xB (%d iters) read latency %f (%d samples)\n", size, iter, (float)perf.cycle / total_node, total_node);

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
