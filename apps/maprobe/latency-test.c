#include "maprobe.h"

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

float test_pointer_tracing_latency(uint64_t size, int step, int iter, int to_csv)
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
    float acpa = (float)perf.cycle / total_node; // average cycle per access
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, acpa, iter, total_node, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) pointer tracing read latency %f, throughput %f B/cycle (%ld samples, %ld cycles)\n",
            size/KB, iter, acpa, total_node * 8 * BYTE / (float)perf.cycle, total_node, perf.cycle
        );
    }
    _perf_g_total_samples += total_node;
    _perf_blackhole(result);
    return acpa;
}

float test_same_address_load_latency(int iter, int to_csv)
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
    float acpa = (float)perf.cycle / total_access; // average cycle per access
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", 0, acpa, iter, total_access, perf.cycle);
    } else {
        printf("same address read latency %f, throughput %f B/cycle (%ld samples, %ld cycles)\n", 
            acpa, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle
        );
    }
    _perf_g_total_samples += total_access;
    _perf_blackhole(result);
    return acpa;
}

float test_read_after_write_latency(int iter, int to_csv)
{
    // printf("same address store-load latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    volatile uint64_t result = 0; // make sure compiler will store data to memory
    // _perf_print_timer();

    _perf_start_timer();
    for (int i = 0; i < iter; i++) {
        uint64_t address = _PERF_TEST_ADDR_BASE;
        result += *((uint64_t*) (address));
        address += sizeof(uint64_t);
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = iter;
    float acpa = (float)perf.cycle / total_access; // average cycle per access
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", 0, acpa, iter, total_access, perf.cycle);
    } else {
        printf("read after write latency %f, throughput %f B/cycle (%ld samples, %ld cycles)\n", 
            acpa, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle
        );
    }
    _perf_g_total_samples += total_access;
    _perf_blackhole(result);
    return acpa;
}

float test_linear_access_latency_simple(uint64_t size, uint64_t step, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    register uint64_t result = 0; 
    uint64_t num_access = size / step;
    // _perf_print_timer();

    _perf_start_timer();
    for (int i = 0; i < iter; i++) {
        uint64_t address = _PERF_TEST_ADDR_BASE;
        for (int j = 0; j < num_access; j++) {
            result += *((volatile uint64_t*) (address));
            address += step;
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = num_access * iter;
    float acpa = (float)perf.cycle / total_access; // average cycle per access
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, acpa, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) simple linear read latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, acpa, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, step
        );
    }
    _perf_g_total_samples += total_access;
    _perf_blackhole(result);
    return acpa;
}

float test_linear_access_latency_batch8(uint64_t size, uint64_t step, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
    uint64_t num_access = size / step;
    num_access += num_access % 8 ? 8 - num_access % 8 : 0;
    assert(num_access >= 8);
    // prepare access offset
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
    for (int i = 0; i < iter; i++) {
        uint64_t address = _PERF_TEST_ADDR_BASE;
        for (int j = 0; j < num_access; j += 8) {
            __asm__ volatile (
                "mv a1, %[addr]\n"
                "add a2, %[addr], %[offset1]\n"
                "add a3, %[addr], %[offset2]\n"
                "add a4, %[addr], %[offset3]\n"
                "add a5, %[addr], %[offset4]\n"
                "add t0, %[addr], %[offset5]\n"
                "add t1, %[addr], %[offset6]\n"
                "add t2, %[addr], %[offset7]\n"
                "ld a0, 0(a1)\n" 
                "ld a0, 0(a2)\n" 
                "ld a0, 0(a3)\n" 
                "ld a0, 0(a4)\n" 
                "ld a0, 0(a5)\n" 
                "ld a0, 0(t0)\n" 
                "ld a0, 0(t1)\n" 
                "ld a0, 0(t2)\n" 
                :: 
                [offset1] "r"(address_offset_1),
                [offset2] "r"(address_offset_2),
                [offset3] "r"(address_offset_3),
                [offset4] "r"(address_offset_4),
                [offset5] "r"(address_offset_5),
                [offset6] "r"(address_offset_6),
                [offset7] "r"(address_offset_7),
                [addr] "r"(address) 
                : "a0", "a1", "a2", "a3", "a4", "a5", "t0", "t1", "t2", "t3"
            );
            address += address_offset_8;
            // register uint64_t access_addr_0 = address + address_offset_0;
            // register uint64_t access_addr_1 = address + address_offset_1;
            // register uint64_t access_addr_2 = address + address_offset_2;
            // register uint64_t access_addr_3 = address + address_offset_3;
            // register uint64_t access_addr_4 = address + address_offset_4;
            // register uint64_t access_addr_5 = address + address_offset_5;
            // register uint64_t access_addr_6 = address + address_offset_6;
            // register uint64_t access_addr_7 = address + address_offset_7;
            // address += address_offset_8;
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_0) : "a0");
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_1) : "a0");
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_2) : "a0");
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_3) : "a0");
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_4) : "a0");
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_5) : "a0");
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_6) : "a0");
            // __asm__ volatile ("ld a0, 0(%[addr])\n" :: [addr] "r"(access_addr_7) : "a0");
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = num_access * iter;
    float acpa = (float)perf.cycle / total_access; // average cycle per access
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, acpa, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) batch(8) linear read latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, acpa, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, step
        );
    }
    _perf_g_total_samples += total_access;
    return acpa;
}

float test_linear_write_latency_batch8(uint64_t size, uint64_t step, int iter, int to_csv)
{
    // printf("stride %d linear access latency test\n", step);
    // printf("range (B), read latency, iters, samples, cycles\n");
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
    for (int i = 0; i < iter; i++) {
        uint64_t address = _PERF_TEST_ADDR_BASE;
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
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_0) : "a0");
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_1) : "a0");
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_2) : "a0");
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_3) : "a0");
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_4) : "a0");
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_5) : "a0");
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_6) : "a0");
            __asm__ volatile ("sd a0, 0(%[addr])\n" :: [addr] "r"(access_addr_7) : "a0");
        }
    }
    _perf_end_timer();
    // _perf_print_timer();
    uint64_t total_access = num_access * iter;
    float acpa = (float)perf.cycle / total_access; // average cycle per access
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", size, acpa, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB (%d iters) batch(8) linear write latency %f, throughput %f B/cycle (%ld samples, %ld cycles), stride %dB\n", 
            size/KB, iter, acpa, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, step
        );
    }
    _perf_g_total_samples += total_access;
    return acpa;
}

float test_linear_access_latency(uint64_t size, uint64_t step, int iter, int to_csv)
{
    return test_linear_access_latency_batch8(size, step, iter, to_csv);
}

float test_linear_write_latency(uint64_t size, uint64_t step, int iter, int to_csv)
{
    return test_linear_write_latency_batch8(size, step, iter, to_csv);
}

float test_random_access_latency(uint64_t num_access, uint64_t test_range, uint64_t test_align, int pregen_addr, int iter, int to_csv)
{
    // printf("align %d random access (cache line) latency test, %s\n",
    //     test_align, pregen_addr ? "use pregen addr array" : "gen rand addr at run time"
    // );
    // printf("range (B), read latency, iters, samples, cycles\n");
    register uint64_t result = 0; 
    // _perf_print_timer();

    uint64_t total_access = num_access * iter;
    if (test_range > total_access*8*_PERF_CACHELINE_SIZE_BYTE) {
        printf("total access size %ldKB less than test range %ldKB, ignored\n",
            total_access*8*_PERF_CACHELINE_SIZE_BYTE/KB,
            test_range/KB
        );
        return 0;
    }

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
    float acpa = (float)perf.cycle / total_access; // average cycle per access
    if (to_csv) {
        printf("%ld, %f, %d, %ld, %ld\n", test_range, acpa, iter, total_access, perf.cycle);
    } else {
        printf("range %ldKB, access %ldKB (cover %ldKB) (%d iters) random read latency %f, throughput %f B/cycle (%ld samples, %ld cycles), align %ldB, %s\n", 
            test_range/KB, total_access*8*BYTE/KB,  total_access*8*_PERF_CACHELINE_SIZE_BYTE/KB, iter, acpa, total_access * 8 * BYTE / (float)perf.cycle, total_access, perf.cycle, test_align,
            pregen_addr ? "pregen addr" : "runtime addr"
        );
    }
    _perf_g_total_samples += total_access;
    _perf_blackhole(result);
    return acpa;
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

void generate_linear_access_latency_matrix(uint64_t step)
{
    // step can be _PERF_CACHELINE_SIZE_BYTE or 8*BYTE
#define LINEAR_ACCESS_MATRIX_SIZE_MAX_POW2_KB 14
    // LINEAR_ACCESS_MATRIX_SIZE_MAX_POW2_KB 14: 14 cases in total, from 1KB to 8MB
    DEFINE_FLOAT_RESULT_MATRIX(linear_access_latency,size_kb_pow2,LINEAR_ACCESS_MATRIX_SIZE_MAX_POW2_KB,iter,3);
    FOR(x,LINEAR_ACCESS_MATRIX_SIZE_MAX_POW2_KB) { linear_access_latency_row_array[x] = x; }
    FOR(x,3) { linear_access_latency_column_array[x] = x; }
    for (int i = 0; i < LINEAR_ACCESS_MATRIX_SIZE_MAX_POW2_KB; i++) {
        int warm_up_iter = i < 6 ? 4 : 1;
        int test_iter = i < 6 ? 4 : 2;
        linear_access_latency_result_array[i][0] = test_linear_access_latency((1<<i)*KB,step,warm_up_iter,0); //warmup
        linear_access_latency_result_array[i][1] = test_linear_access_latency((1<<i)*KB,step,test_iter,0); //test
        linear_access_latency_result_array[i][2] = test_linear_access_latency((1<<i)*KB,step,test_iter,0); //test
    }
    printf("[test step %ld]\n", step);
    print_float_result_matrix(&linear_access_latency_matrix_meta);
}

void generate_pointer_tracing_latency_matrix(uint64_t step)
{
    // step can be _PERF_CACHELINE_SIZE_BYTE or 8*BYTE
#define POINTER_CHASING_MATRIX_SIZE_MAX_POW2_KB 14
    // POINTER_CHASING_MATRIX_SIZE_MAX_POW2_KB 14: 14 cases in total, from 1KB to 8MB
    DEFINE_FLOAT_RESULT_MATRIX(pointer_tracing_latency,size_kb_pow2,POINTER_CHASING_MATRIX_SIZE_MAX_POW2_KB,iter,3);
    FOR(x,POINTER_CHASING_MATRIX_SIZE_MAX_POW2_KB) { pointer_tracing_latency_row_array[x] = x; }
    FOR(x,3) { pointer_tracing_latency_column_array[x] = x; }
    for (int i = 0; i < POINTER_CHASING_MATRIX_SIZE_MAX_POW2_KB; i++) {
        int warm_up_iter = i < 6 ? 4 : 1;
        int test_iter = i < 6 ? 4 : 2;
        pointer_tracing_latency_result_array[i][0] = test_pointer_tracing_latency((1<<i)*KB,step,warm_up_iter,0); //warmup
        pointer_tracing_latency_result_array[i][1] = test_pointer_tracing_latency((1<<i)*KB,step,test_iter,0); //test
        pointer_tracing_latency_result_array[i][2] = test_pointer_tracing_latency((1<<i)*KB,step,test_iter,0); //test
    }
    printf("[test step %ld]\n", step);
    print_float_result_matrix(&pointer_tracing_latency_matrix_meta);
}

void generate_random_access_latency_matrix()
{
#define RANDOM_ACCESS_MATRIX_SIZE_MAX_POW2_KB 6
    // RANDOM_ACCESS_MATRIX_SIZE_MAX_POW2_KB 10: from 1KB to 512KB
#define RANDOM_ACCESS_MATRIX_ACCESS_MAX_POW2_KB 6
    // RANDOM_ACCESS_MATRIX_ACCESS_MAX_POW2_KB 10: from 1KB to 512KB
    DEFINE_FLOAT_RESULT_MATRIX(random_access_latency,test_range_size_kb_pow2,RANDOM_ACCESS_MATRIX_SIZE_MAX_POW2_KB,access_size_kb_pow2,RANDOM_ACCESS_MATRIX_ACCESS_MAX_POW2_KB);
    FOR(x,RANDOM_ACCESS_MATRIX_SIZE_MAX_POW2_KB) { random_access_latency_row_array[x] = x; }
    FOR(x,RANDOM_ACCESS_MATRIX_ACCESS_MAX_POW2_KB) { random_access_latency_column_array[x] = x; }
    for (int i = 0; i < RANDOM_ACCESS_MATRIX_SIZE_MAX_POW2_KB; i++) {
        for (int j = 0; j < RANDOM_ACCESS_MATRIX_ACCESS_MAX_POW2_KB; j++) {
            uint64_t access_size = (1<<j)*KB;
            uint64_t num_access = access_size / sizeof(uint64_t);
            uint64_t test_range = (1<<i)*KB;
            test_random_access_latency(num_access, test_range, sizeof(uint64_t), 1, 1, 0); //warmup
            random_access_latency_result_array[i][j] = test_random_access_latency(num_access, test_range, sizeof(uint64_t), 1, 1, 0); //test
        }
    }
    print_float_result_matrix(&random_access_latency_matrix_meta);
}
