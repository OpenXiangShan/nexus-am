#include <klib.h>
#include "maprobe.h"

int main()
{
    _perf_calibrate();
    printf("Memory throughput:\n");
    test_mem_throughput(512);
    printf("L1 latency:\n");
    test_latency(4 * KB, 5);
    test_latency(_PERF_L1_NOALIAS_SIZE_BYTE, 2);
    test_latency(_PERF_L1_SIZE_BYTE/2, 2);
    test_latency(_PERF_L1_SIZE_BYTE, 2);
    printf("L2 latency:\n");
    test_latency(_PERF_L2_SIZE_BYTE/2, 2);
    // test_latency(_PERF_L2_SIZE_BYTE, 2);
    printf("L3 latency:\n");
    test_latency(_PERF_L3_SIZE_BYTE/2, 2);
    // test_latency(_PERF_L3_SIZE_BYTE,2);
    // printf("MEM:\n");
    // test_latency(_PERF_L3_SIZE_BYTE*2,2);

    return 0;
}