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
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 10, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 100, 0);
    printf("L2 (L1 same set) linear cache line load:\n");
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 2, 0);
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L1_SAME_SET, 4, 0);
    printf("L1 (L2 same slice) linear cache line load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 1, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 2, 0);
    printf("L2 (L2 same slice) linear cache line load:\n");
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 1, 0);
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_L2_SAME_SLICE, 2, 0);
    printf("L1 (page traverse) linear cache line load:\n");
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 10, 0);
    test_linear_access_latency(_PERF_L1_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 100, 0);
    printf("L2 (page traverse) linear cache line load:\n");
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 2, 0);
    test_linear_access_latency(_PERF_L2_SIZE_BYTE, _PERF_ADDR_STRIDE_NEXT_PAGE, 4, 0);
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
    printf("dobule word by dobule word tracing:\n");
    test_pointer_tracing_latency(_PERF_PAGE_SIZE_BYTE, 8*BYTE, 10, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE/2, 8*BYTE, 2, 0);
    test_pointer_tracing_latency(_PERF_L1_SIZE_BYTE, 8*BYTE, 2, 0);
    test_pointer_tracing_latency(_PERF_L2_SIZE_BYTE/2, 8*BYTE, 2, 0);
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
    printf("load then store to the same address:\n");
    test_read_after_write_latency(1024, 0);
    test_read_after_write_latency(1024, 0);
    test_read_after_write_latency(1024, 0);
    // more to be added
}

void typical_l1_access_test_set()
{
    printf("------------- typical dcache access pattern test set -------------\n");
    printf("ideal load bandwidth:\n");
    test_l1_load_bandwidth(_PERF_L1_SIZE_BYTE, 2, 0);
    test_l1_load_bandwidth(_PERF_L1_SIZE_BYTE, 10, 0);
    printf("ideal store bandwidth:\n");
    test_l1_store_bandwidth(_PERF_L1_SIZE_BYTE, 2, 0);
    test_l1_store_bandwidth(_PERF_L1_SIZE_BYTE, 10, 0);
    printf("ideal write combine buffer bandwidth:\n");
    test_l1_store_wcb_bandwidth(_PERF_L1_SIZE_BYTE, 2, 0);
    test_l1_store_wcb_bandwidth(_PERF_L1_SIZE_BYTE, 5, 0);
    printf("replacement error penalty:\n");
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*32,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*32,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
}

// typical latency test for fast regression
void typical_latency_test()
{
    _perf_g_total_samples = 0;
    typical_l1_access_test_set();
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
    test_l1_load_bandwidth(4*KB, 5, 0);
    test_l1_load_bandwidth(4*KB, 5, 0);
    test_l1_store_bandwidth(4*KB, 5, 0);
    test_l1_store_bandwidth(4*KB, 5, 0);
    test_l1_store_wcb_bandwidth(8*KB, 5, 0);
    test_l1_store_wcb_bandwidth(8*KB, 5, 0);
    test_pointer_tracing_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 5, 0);
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, sizeof(uint64_t), 5, 0);
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, sizeof(uint64_t), 5, 0);
    test_linear_access_latency(_PERF_PAGE_SIZE_BYTE, _PERF_CACHELINE_SIZE_BYTE, 5, 0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*4,_PERF_ADDR_STRIDE_L1_SAME_SET,8,0);
    test_random_access_latency(4096, 1024*MB, _PERF_CACHELINE_SIZE_BYTE, 0, 1, 0);
    test_random_access_latency(4096, 1024*MB, _PERF_CACHELINE_SIZE_BYTE, 1, 1, 0);
    test_same_address_load_latency(1024, 0);
    test_read_after_write_latency(1024, 0);
    printf("total samples: %ld\n", _perf_g_total_samples);
}

void l2_l3_pressure_test()
{
    _perf_calibrate();
    printf("L2 and L3 same set pressure test:\n");
    for (int i = 1; i < 16; i++) {
        printf("ways accessed: %d\n", i);
        test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L2_SAME_SET*i,_PERF_ADDR_STRIDE_L2_SAME_SET,64,0);
        test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L2_SAME_SET*i,_PERF_ADDR_STRIDE_L2_SAME_SET,64,0);
    }
    for (int i = 16; i <= 512; i*=2) {
        printf("ways accessed: %d\n", i);
        // jump at i = 32
        test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L2_SAME_SET*i,_PERF_ADDR_STRIDE_L2_SAME_SET,64,0);
    }

    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*32,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*32,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*64,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*64,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    // jump at i = 128
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*128,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*128,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*256,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*256,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*512,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
    test_linear_access_latency_simple(_PERF_ADDR_STRIDE_L1_SAME_SET*512,_PERF_ADDR_STRIDE_L1_SAME_SET,64,0);
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
    printf("total sampl8es: %ld\n", _perf_g_total_samples);
}

int main()
{
    latency_test_example();

    generate_linear_access_latency_matrix(8*BYTE);
    generate_linear_access_latency_matrix(_PERF_CACHELINE_SIZE_BYTE);
    generate_pointer_tracing_latency_matrix(8*BYTE);
    generate_pointer_tracing_latency_matrix(_PERF_CACHELINE_SIZE_BYTE);
    // generate_random_access_latency_matrix();
    generate_replacement_test_matrix();

    // matrix_print_example();
    typical_latency_test();
    // pointer_tracing_graph();
    // latency_test();
    // legacy_latency_throughput_test();
    l2_l3_pressure_test();
    return 0;
    // return 0;
}