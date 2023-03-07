#include "maprobe.h"

// inline uint64_t get_next_linear_address(uint64_t current_addr, uint64_t step) {
//     return current_addr + step;
// }

inline uint64_t generate_rand_address(uint64_t base_addr, uint64_t end_addr, uint64_t align) {
    return (rand() % (end_addr - base_addr) + base_addr) / align * align;
}

void generate_rand_address_array(uint64_t* dest, uint64_t base_addr, uint64_t end_addr, uint64_t align, int number) {
    for (int i = 0; i < number; i++) {
        *(dest + i) = generate_rand_address(base_addr, end_addr, align);
    }
}

uint64_t generate_pointer_tracing_address(uint64_t base_addr, uint64_t end_addr, uint64_t step) {
    return setup_pointer_tracing_linklist(base_addr, end_addr, step);
}

uint64_t setup_pointer_tracing_linklist(uint64_t base_addr, uint64_t end_addr, uint64_t step)
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

uint64_t read_pointer_tracing_linklist(uint64_t base_addr, uint64_t num_valid_node)
{
    uint64_t cur_addr = base_addr;
    for (int i = 0; i < num_valid_node; i++) {
        cur_addr = (*(uint64_t*)cur_addr);
    }
    return cur_addr;
}

void latency_test_warmup(uint64_t base_addr, uint64_t end_addr)
{
    setup_pointer_tracing_linklist(base_addr, end_addr, _PERF_CACHELINE_SIZE_BYTE);
}

void test_pointer_tracing_latency(uint64_t size, int step, int iter, int to_csv)
{
    // printf("pointer tracing latency test\n");
    // printf("range (B), read latency, iters, samples, cycles\n");
    register uint64_t result = 0; // make sure compiler will not opt read_pointer_tracing_linklist
    _perf_start_timer();
    uint64_t nnode = setup_pointer_tracing_linklist(_PERF_TEST_ADDR_BASE, _PERF_TEST_ADDR_BASE + size, step);
    _perf_end_timer();
    uint64_t total_node = nnode * iter;
    // _perf_print_timer();

    _perf_start_timer();
    for (int i = 0; i < iter; i++) {
        result += read_pointer_tracing_linklist(_PERF_TEST_ADDR_BASE, nnode);
    }
    _perf_end_timer();
    // _perf_print_timer();
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_node, iter, total_node, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) pointer tracing read latency %f, throughput %f B/cycle (%ld samples, %ld cycles)\n",
            size/KB, iter, (float)perf.cycle / total_node, total_node * 8 * BYTE / (float)perf.cycle, total_node, perf.cycle
        );
    }

    _perf_blackhole(result);
    _perf_g_total_samples += total_node;
}

void test_same_address_load_latency(int iter, int to_csv)
{
    // printf("same address load latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    register uint64_t result = 0; 
    // _perf_print_timer();

    _perf_start_timer();
    uint64_t address = _PERF_TEST_ADDR_BASE;
    for (int i = 0; i < iter; i++) {
        result += *((volatile uint64_t*) (address));
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = iter;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", 0, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("same address read latency %f, throughput %f B/cycle (%ld samples, %ld cycles)\n", 
            (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle
        );
    }

    _perf_blackhole(result);
    _perf_g_total_samples += total_access;
}

void test_read_after_write_latency(int iter, int to_csv)
{
    // printf("same address store-load latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    volatile uint64_t result = 0; // make sure compiler will store data to memory
    // _perf_print_timer();

    _perf_start_timer();
    uint64_t address = _PERF_TEST_ADDR_BASE;
    for (int i = 0; i < iter; i++) {
        result += *((uint64_t*) (address));
        address += sizeof(uint64_t);
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = iter;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", 0, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("read after write latency %f, throughput %f B/cycle (%ld samples, %ld cycles)\n", 
            (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle
        );
    }

    _perf_blackhole(result);
    _perf_g_total_samples += total_access;
}

void test_linear_access_latency_simple(uint64_t size, uint64_t step, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    register uint64_t result = 0; 
    uint64_t num_access = size / step;
    // _perf_print_timer();

    _perf_start_timer();
    uint64_t address = _PERF_TEST_ADDR_BASE;
    for (int i = 0; i < iter; i++) {
        for (int j = 0; j < num_access; j++) {
            result += *((uint64_t*) (address));
            address += step;
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = num_access * iter;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) simple linear read latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, step
        );
    }

    _perf_blackhole(result);
    _perf_g_total_samples += total_access;
}

void test_linear_access_latency_batch8(uint64_t size, uint64_t step, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    register uint64_t result = 0; 
    uint64_t num_access = size / step;
    num_access += num_access % 8 ? 8 - num_access % 8 : 0;
    assert(num_access >= 8);
    // prepare access offset
    uint64_t address_offset_0 = 0;
    register uint64_t address_offset_1 = step * 1;
    register uint64_t address_offset_2 = step * 2;
    register uint64_t address_offset_3 = step * 3;
    register uint64_t address_offset_4 = step * 4;
    register uint64_t address_offset_5 = step * 5;
    register uint64_t address_offset_6 = step * 6;
    register uint64_t address_offset_7 = step * 7;
    register uint64_t address_offset_8 = step * 8;

    // _perf_print_timer();
    _perf_start_timer();
    uint64_t address = _PERF_TEST_ADDR_BASE;
    for (int i = 0; i < iter; i++) {
        for (int j = 0; j < num_access; j += 8) {
            register uint64_t access_addr_0 = address + address_offset_0;
            register uint64_t access_addr_1 = address + address_offset_1;
            register uint64_t access_addr_2 = address + address_offset_2;
            register uint64_t access_addr_3 = address + address_offset_3;
            register uint64_t access_addr_4 = address + address_offset_4;
            register uint64_t access_addr_5 = address + address_offset_5;
            register uint64_t access_addr_6 = address + address_offset_6;
            register uint64_t access_addr_7 = address + address_offset_7;
            address += address_offset_8;
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_0) : "a0");
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_1) : "a0");
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_2) : "a0");
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_3) : "a0");
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_4) : "a0");
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_5) : "a0");
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_6) : "a0");
            __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_7) : "a0");
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = num_access * iter;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) batch(8) linear read latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, step
        );
    }

    _perf_blackhole(result);
    _perf_g_total_samples += total_access;
}

void test_linear_access_latency(uint64_t size, uint64_t step, int iter, int to_csv)
{
    test_linear_access_latency_batch8(size, step, iter, to_csv);
}

void test_l1_load_bandwidth(uint64_t size, int iter, int to_csv)
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
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) dcache linear (8Byte) read, latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, 8
        );
    }
    _perf_g_total_samples += total_access;
}

void test_l1_store_bandwidth(uint64_t size, int iter, int to_csv)
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
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) dcache linear (8Byte) store latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, 8
        );
    }
    _perf_g_total_samples += total_access;
}

void test_l1_store_wcb_bandwidth(uint64_t size, int iter, int to_csv)
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
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) dcache linear (8Byte) store latency %f, throughput %f B/cycle (L1-L2 %f B/cycle) (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, (float)perf.cycle / total_access, total_access * 8 * BYTE / (float)perf.cycle, total_access * _PERF_CACHELINE_SIZE_BYTE / (float)perf.cycle, total_access, perf.cycle, 8
        );
    }
    _perf_g_total_samples += total_access;
}

void test_random_access_latency(uint64_t num_access, uint64_t test_range, uint64_t test_align, int pregen_addr, int iter, int to_csv)
{
    // printf("align %d random access (cache line) latency test, %s\n",
    //     test_align, pregen_addr ? "use pregen addr array" : "gen rand addr at run time"
    // );
    // printf("range (B), read latency, iters, samples, cycles\n");
    register uint64_t result = 0; 
    // _perf_print_timer();

    // alloc memory for random access addr array and data
    assert(test_align >= 8 * BYTE);
    // assert(size >= test_align);
    // uint64_t num_access = size / test_align;
    if (pregen_addr) {
        uint64_t test_array_base_addr = _PERF_TEST_ADDR_BASE + num_access * sizeof(uint64_t*);
        uint64_t address_array_base_addr = _PERF_TEST_ADDR_BASE;
        generate_rand_address_array((uint64_t*)address_array_base_addr, test_array_base_addr, test_array_base_addr + test_range, test_align, num_access);
        _perf_start_timer();
        for (int i = 0; i < iter; i++) {
            for (int j = 0; j < num_access; j++) {
                result += *((uint64_t*) (address_array_base_addr + j * sizeof(uint64_t*)));
            }
        }
        _perf_end_timer();
    } else {
        _perf_start_timer();
        for (int i = 0; i < iter; i++) {
            for (int j = 0; j < num_access; j++) {
                result += *((uint64_t*) (generate_rand_address(_PERF_TEST_ADDR_BASE, _PERF_TEST_ADDR_BASE + test_range, test_align)));
            }
        }
        _perf_end_timer();
    }
    // _perf_print_timer();
    uint64_t total_access = num_access * iter;
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", test_range, (float)perf.cycle / total_access, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB, access cover %ldKB (%d iters) random read latency %f, throughput %f B/cycle (%ld samples, %ld cycles), align %ldB, %s\n", 
            test_range/KB, total_access*8*_PERF_CACHELINE_SIZE_BYTE/KB, iter, (float)perf.cycle / (total_access), total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, test_align,
            pregen_addr ? "pregen addr" : "runtime addr"
        );
    }

    _perf_blackhole(result);
    _perf_g_total_samples += total_access;
}

void legacy_test_mem_throughput(uint64_t iter)
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

void legacy_test_mem_throughput_same_set(uint64_t iter)
{
    uint64_t remain = iter;
    uint64_t result = 0;
    uint64_t access_addr = _PERF_TEST_ADDR_BASE;
    _perf_start_timer();
    while (remain--) {
        result += *(uint64_t*) access_addr;
        access_addr += _PERF_ADDR_STRIDE_L1_SAME_SET;
    }
    _perf_end_timer();
    *(uint64_t*) _PERF_BLACKHOLE = result;
    printf("mem band width %f B/cycle (%d samples)\n", (float)iter * _PERF_CACHELINE_SIZE_BYTE / perf.cycle, iter);
}
