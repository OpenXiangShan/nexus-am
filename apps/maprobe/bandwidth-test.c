#include "maprobe.h"

float test_l1_load_bandwidth(uint64_t size, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    assert(size >= _PERF_CACHELINE_SIZE_BYTE);

    // _perf_print_timer();
    _perf_start_timer();
    for (int i = 0; i < iter; i++) {
        for (uint64_t address = _PERF_TEST_ADDR_BASE; address < _PERF_TEST_ADDR_BASE + size; address += _PERF_CACHELINE_SIZE_BYTE) {
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("ld a0, 8(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("ld a0, 16(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("ld a0, 24(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("ld a0, 32(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("ld a0, 40(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("ld a0, 48(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("ld a0, 56(%[addr])\n" :: [addr] "r"(address) : "a0");
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = size / _PERF_CACHELINE_SIZE_BYTE * 8 * iter;
    float bandwidth = total_access * 8 * BYTE / (float)perf.cycle;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) dcache linear (8Byte) read, latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, bandwidth, total_access, perf.cycle, 8
        );
    }
    _perf_g_total_samples += total_access;
    return bandwidth;
}

float test_l1_store_bandwidth(uint64_t size, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    assert(size >= _PERF_CACHELINE_SIZE_BYTE);

    // _perf_print_timer();
    _perf_start_timer();
    for (int i = 0; i < iter; i++) {
        for (uint64_t address = _PERF_TEST_ADDR_BASE; address < _PERF_TEST_ADDR_BASE + size; address += _PERF_CACHELINE_SIZE_BYTE) {
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("sd a0, 8(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("sd a0, 16(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("sd a0, 24(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("sd a0, 32(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("sd a0, 40(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("sd a0, 48(%[addr])\n" :: [addr] "r"(address) : "a0");
            __asm__ volatile ("sd a0, 56(%[addr])\n" :: [addr] "r"(address) : "a0");
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = size / _PERF_CACHELINE_SIZE_BYTE * 8 * iter;
    float bandwidth = total_access * 8 * BYTE / (float)perf.cycle;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) dcache linear (8Byte) store latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, bandwidth, total_access, perf.cycle, 8
        );
    }
    _perf_g_total_samples += total_access;
    return bandwidth;
}

float test_l1_store_wcb_bandwidth(uint64_t size, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    assert(size >= _PERF_CACHELINE_SIZE_BYTE);

    // _perf_print_timer();
    _perf_start_timer();
    for (int i = 0; i < iter; i++) {
        for (uint64_t address = _PERF_TEST_ADDR_BASE; address < _PERF_TEST_ADDR_BASE + size; address += _PERF_CACHELINE_SIZE_BYTE) {
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(address) : "a0");
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = size / _PERF_CACHELINE_SIZE_BYTE * iter;
    float bandwidth = total_access * _PERF_CACHELINE_SIZE_BYTE / (float)perf.cycle;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) dcache linear (8Byte) store latency %f, throughput %f B/cycle (L1-L2 %f B/cycle) (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access * _PERF_CACHELINE_SIZE_BYTE / (float)perf.cycle, total_access, perf.cycle, _PERF_CACHELINE_SIZE_BYTE
        );
    }
    _perf_g_total_samples += total_access;
    return bandwidth;
}