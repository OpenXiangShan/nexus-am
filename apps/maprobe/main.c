#include <klib.h>
#include "maprobe.h"

void typical_linear_load_test_set()
{
    _perf_calibrate();
    printf("------------- linear load test set -------------\n");
    printf("page size linear double word load:\n");
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, sizeof(uint64_t), 1, 0);
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, sizeof(uint64_t), 2, 0);
    printf("page size linear cache line load:\n");
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 1, 0);
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    printf("dcache/2 linear double word load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE / 2, sizeof(uint64_t), 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE / 2, sizeof(uint64_t), 2, 0);
    printf("dcache/2 linear cache line load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE / 2, _PERF_CACHELINE_SIZE_BYTE, 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE / 2, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    printf("dcache linear double word load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, sizeof(uint64_t), 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, sizeof(uint64_t), 2, 0);
    printf("dcache linear cache line load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    printf("L2 linear cache line load:\n");
    test_linear_access_latency(_PERF_L2_SIZE_BYTE / 2, _PERF_CACHELINE_SIZE_BYTE, 1, 0);
    test_linear_access_latency(_PERF_L2_SIZE_BYTE / 2, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    printf("L1 (L1 same set) linear cache line load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 10, 0);
    printf("L2 (L1 same set) linear cache line load:\n");
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 1, 0);
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 2, 0);
    printf("L1 (L2 same slice) linear cache line load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 2, 0);
    printf("L2 (L2 same slice) linear cache line load:\n");
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 1, 0);
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 2, 0);
    printf("L1 (page traverse) linear cache line load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 10, 0);
    printf("L2 (page traverse) linear cache line load:\n");
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 1, 0);
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 2, 0);
    printf("total samples: %ld\n", _perf_g_total_samples);
}

void typical_random_load_test_set()
{
    printf("------------- random load test set -------------\n");
    printf("from page size random load (word):\n");
    test_random_access_latency(1024, _PERF_PAGE_SIZE_BYTE, 8*BYTE, 1, 1, 0);
    test_random_access_latency(1024, _PERF_PAGE_SIZE_BYTE, 8*BYTE, 0, 1, 0);
    printf("from page size random load (cache line):\n");
    test_random_access_latency(1024, _PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 1, 1, 0);
    test_random_access_latency(1024, _PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 0, 1, 0);
    printf("from dcache/2 size random load (word):\n");
    test_random_access_latency(1024, _PERF_L1_SIZE_BYTE/2, 8*BYTE, 1, 1, 0);
    test_random_access_latency(1024, _PERF_L1_SIZE_BYTE/2, 8*BYTE, 0, 1, 0);
    printf("from dcache/2 size random load (cache line):\n");
    test_random_access_latency(1024, _PERF_L1_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 1, 1, 0);
    test_random_access_latency(1024, _PERF_L1_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 0, 1, 0);
    printf("from dcache size random load (word):\n");
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L1_SIZE_BYTE, 8*BYTE, 1, 1, 0);
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L1_SIZE_BYTE, 8*BYTE, 0, 1, 0);
    printf("from dcache size random load (cache line):\n");
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L1_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 1, 1, 0);
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L1_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 0, 1, 0);
    printf("from l2 size random load (word):\n");
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L2_SIZE_BYTE, 8*BYTE, 1, 1, 0);
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L2_SIZE_BYTE, 8*BYTE, 0, 1, 0);
    printf("from l2 size random load (cache line):\n");
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L2_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 1, 1, 0);
    test_random_access_latency(_PERF_L1_SIZE_BYTE/_PERF_CACHELINE_SIZE_BYTE*2, _PERF_L2_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 0, 1, 0);
    printf("total samples: %ld\n", _perf_g_total_samples);
}

void typical_pointer_tracing_load_test_set()
{
    printf("------------- pointer tracing load test set -------------\n");
    printf("cacheline by cacheline tracing:\n");
    test_pointer_tracing_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 10, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    test_pointer_tracing_latency(_PERF_L2_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    test_pointer_tracing_latency(_PERF_L2_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 1, 0);
    test_pointer_tracing_latency(_PERF_L3_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 1, 0);
    printf("page by page, tracing:\n");
    test_pointer_tracing_latency(_PERF_PAGE_SIZE_BYTE*2, _PERF_PAGE_SIZE_BYTE, 10, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE/2, _PERF_PAGE_SIZE_BYTE, 10, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE, _PERF_PAGE_SIZE_BYTE, 10, 0);
    test_pointer_tracing_latency(_PERF_L2_SIZE_BYTE/2, _PERF_PAGE_SIZE_BYTE, 10, 0);
    test_pointer_tracing_latency(_PERF_L2_SIZE_BYTE, _PERF_PAGE_SIZE_BYTE, 10, 0);
    printf("total samples: %ld\n", _perf_g_total_samples);
}

void typical_memory_disambiuation_test_set()
{
    printf("------------- memory disambiuation test set -------------\n");
    printf("load from the same address:\n");
    test_same_address_load_latency(1024, 0);
    test_same_address_load_latency(1024, 0);
    test_same_address_load_latency(1024, 0);
    // more to be added
}

// typical latency test for fast regression
void typical_latency_test()
{
    _perf_g_total_samples = 0;
    typical_linear_load_test_set();
    typical_random_load_test_set();
    typical_pointer_tracing_load_test_set();
    typical_memory_disambiuation_test_set();
}

void pointer_tracing_graph()
{
    _perf_g_total_samples = 0;
    _perf_calibrate();
    printf("data for pointer tracing latency graph:\n");
    printf("range (B), read latency, iters, samples\n");
    for (int i = 1*KB; i < 64*KB; i = i + 1*KB) {
        test_pointer_tracing_latency(i, _PERF_CACHELINE_SIZE_BYTE, 2, 1);
    }
    for (int i = 64*KB; i < 1024*KB; i = i + 64*KB) {
        test_pointer_tracing_latency(i, _PERF_CACHELINE_SIZE_BYTE, 1, 1);
    }
    test_pointer_tracing_latency(1024*KB, _PERF_CACHELINE_SIZE_BYTE, 1, 1);
    for (int i = 1*MB; i <8*MB; i = i + 1*MB) {
        test_pointer_tracing_latency(i, _PERF_CACHELINE_SIZE_BYTE, 1, 1);
    }
    printf("total samples: %ld\n", _perf_g_total_samples);
}

// a simple test set used to check if test is working correctly
void latency_test_example()
{
    _perf_calibrate();
    printf("latency test example:\n");
    test_pointer_tracing_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 5, 0);
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 5, 0);
    test_random_access_latency(4096, 1024*MB, _PERF_CACHELINE_SIZE_BYTE, 0, 1, 0);
    test_random_access_latency(4096, 1024*MB, _PERF_CACHELINE_SIZE_BYTE, 1, 1, 0);
    test_same_address_load_latency(1024, 0);
    printf("total samples: %ld\n", _perf_g_total_samples);
}

void legacy_latency_throughput_test()
{
    _perf_calibrate();
    printf("Memory throughput:\n");
    legacy_test_mem_throughput(1024);
    printf("L1 latency:\n");
    test_pointer_tracing_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 5, 0);
    test_pointer_tracing_latency(_PERF_L1_NOALIAS_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    printf("L2 latency:\n");
    test_pointer_tracing_latency(_PERF_L2_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    // test_pointer_tracing_latency(_PERF_L2_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    printf("L3 latency:\n");
    test_pointer_tracing_latency(_PERF_L3_SIZE_BYTE/2, _PERF_CACHELINE_SIZE_BYTE, 2, 0);
    // test_pointer_tracing_latency(_PERF_L3_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE,2, 0);
    // printf("MEM:\n");
    // test_pointer_tracing_latency(_PERF_L3_SIZE_BYTE*2, _PERF_CACHELINE_SIZE_BYTE,2, 0);
    printf("total samples: %ld\n", _perf_g_total_samples);
}

int main()
{
    latency_test_example();
    typical_latency_test();
    // pointer_tracing_graph();
    // latency_test();
    // legacy_latency_throughput_test();
    return 0;
}