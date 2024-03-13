#include "spb_test.h"

static uint64_t seed = 0;

void spb_memset(uint64_t start, uint64_t range, uint64_t value) {
    uint64_t start_addr = start;
    uint64_t end_addr = start_addr + range;
    const uint64_t stride = 8;
    printf("sequential memset: range 0x%x bytes starts\n", range);
    uint64_t cycle_start = csr_read(CSR_MCYCLE);
    uint64_t instr_start = csr_read(CSR_MINSTRET);
    while(start_addr < end_addr) {
        for(uint64_t i=0; i<stride; i++) {
            // printf("addr: %p\n", ((uint64_t*)(start_addr) + i));
            *((uint64_t*)(start_addr) + i) = value;
        }
        start_addr += stride * sizeof(uint64_t);
    }
    uint64_t cycle_end = csr_read(CSR_MCYCLE);
    uint64_t instr_end = csr_read(CSR_MINSTRET);
    printf("sequential memset: range 0x%x bytes ends\n", range);
    printf("IPC in memset: %.5f\n", (float)(instr_end - instr_start) / (cycle_end - cycle_start));
}

void spb_memcopy(uint64_t source, uint64_t dest, uint64_t range) {
    uint64_t start_addr_dest = dest;
    uint64_t end_addr_dest = start_addr_dest + range;
    uint64_t start_addr_source = source;
    const uint64_t stride = 8;
    printf("sequential memcopy: range 0x%x bytes starts\n", range);
    uint64_t cycle_start = csr_read(CSR_MCYCLE);
    uint64_t instr_start = csr_read(CSR_MINSTRET);
    while(start_addr_dest < end_addr_dest) {
        for(uint64_t i=0; i<stride; i++) {
            // printf("addr: %p\n", ((uint64_t*)(start_addr) + i));
            *((uint64_t*)(start_addr_dest) + i) = *((uint64_t*)(start_addr_source) + i);
            // *((uint64_t*)(dest)) = *((uint64_t*)(start_addr_source) + i);
        }
        start_addr_dest += stride * sizeof(uint64_t);
        start_addr_source += stride * sizeof(uint64_t);
    }
    uint64_t cycle_end = csr_read(CSR_MCYCLE);
    uint64_t instr_end = csr_read(CSR_MINSTRET);
    printf("sequential memcopy: range 0x%x bytes ends\n", range);
    printf("IPC in memcopy: %.5f\n", (float)(instr_end - instr_start) / (cycle_end - cycle_start));
}

void spb_memcopy_decr(uint64_t source, uint64_t dest, uint64_t range) {
    uint64_t start_addr_dest = dest;
    uint64_t end_addr_dest = start_addr_dest - range;
    uint64_t start_addr_source = source;
    const uint64_t stride = 8;
    printf("sequential memcopy: range 0x%x bytes starts\n", range);
    uint64_t cycle_start = csr_read(CSR_MCYCLE);
    uint64_t instr_start = csr_read(CSR_MINSTRET);
    while(start_addr_dest > end_addr_dest) {
        for(uint64_t i=0; i<stride; i++) {
            // printf("addr: %p\n", ((uint64_t*)(start_addr) + i));
            *((uint64_t*)(start_addr_dest) - i) = *((uint64_t*)(start_addr_source) - i);
            // *((uint64_t*)(dest)) = *((uint64_t*)(start_addr_source) - i);
        }
        start_addr_dest -= stride * sizeof(uint64_t);
        start_addr_source -= stride * sizeof(uint64_t);
    }
    uint64_t cycle_end = csr_read(CSR_MCYCLE);
    uint64_t instr_end = csr_read(CSR_MINSTRET);
    printf("sequential memcopy: range 0x%x bytes ends\n", range);
    printf("IPC in memcopy: %.5f\n", (float)(instr_end - instr_start) / (cycle_end - cycle_start));
}

void spb_memcopy_muti_stream(uint64_t source, uint64_t dest, uint64_t range, uint64_t stream_depth) {
    uint64_t start_addr_dest = dest;
    uint64_t end_addr_dest = start_addr_dest + range;
    uint64_t start_addr_source = source;
    const uint64_t stride = 8;
    printf("sequential memcopy muti stream: range 0x%x bytes starts\n", range);
    uint64_t cycle_start = csr_read(CSR_MCYCLE);
    uint64_t instr_start = csr_read(CSR_MINSTRET);
    while(start_addr_dest < end_addr_dest) {
        for(uint64_t i=0; i<stride; i++) {
            // printf("addr: %p\n", ((uint64_t*)(start_addr) + i));
            *((uint64_t*)(start_addr_dest) + i) = *((uint64_t*)(start_addr_source) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 1) + i) = *((uint64_t*)(start_addr_source + stream_depth * 1) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 2) + i) = *((uint64_t*)(start_addr_source + stream_depth * 2) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 3) + i) = *((uint64_t*)(start_addr_source + stream_depth * 3) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 4) + i) = *((uint64_t*)(start_addr_source + stream_depth * 4) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 5) + i) = *((uint64_t*)(start_addr_source + stream_depth * 5) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 6) + i) = *((uint64_t*)(start_addr_source + stream_depth * 6) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 7) + i) = *((uint64_t*)(start_addr_source + stream_depth * 7) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 8) + i) = *((uint64_t*)(start_addr_source + stream_depth * 8) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 9) + i) = *((uint64_t*)(start_addr_source + stream_depth * 9) + i);
            *((uint64_t*)(start_addr_dest + stream_depth * 10) + i) = *((uint64_t*)(start_addr_source + stream_depth * 10) + i);
        }
        start_addr_dest += stride * sizeof(uint64_t);
        start_addr_source += stride * sizeof(uint64_t);
    }
    uint64_t cycle_end = csr_read(CSR_MCYCLE);
    uint64_t instr_end = csr_read(CSR_MINSTRET);
    printf("sequential memcopy muti stream: range 0x%x bytes ends\n", range);
    printf("IPC in memcopy: %.5f\n", (float)(instr_end - instr_start) / (cycle_end - cycle_start));
}

void spb_memcopy_muti_stream_hand_write(uint64_t source, uint64_t dest, uint64_t range) {
    uint64_t start_addr_dest = dest;
    uint64_t end_addr_dest = start_addr_dest + range;
    uint64_t start_addr_source = source;
    const uint64_t stride = 8;
    printf("sequential memcopy muti stream hand write: range 0x%x bytes starts\n", range);
    uint64_t cycle_start = csr_read(CSR_MCYCLE);
    uint64_t instr_start = csr_read(CSR_MINSTRET);
    while(start_addr_dest < end_addr_dest) {
        for(uint64_t i=0; i<stride; i++) {
            // printf("addr: %p\n", ((uint64_t*)(start_addr) + i));
            *((uint64_t*)(start_addr_dest) + i) = *((uint64_t*)(start_addr_source) + i);
            *((uint64_t*)(start_addr_dest + 400000) + i) = *((uint64_t*)(start_addr_source + 400000) + i);
            *((uint64_t*)(start_addr_dest + 402496) + i) = *((uint64_t*)(start_addr_source + 402496) + i);
            *((uint64_t*)(start_addr_dest + 802496) + i) = *((uint64_t*)(start_addr_source + 802496) + i);
            *((uint64_t*)(start_addr_dest + 1202496) + i) = *((uint64_t*)(start_addr_source + 1202496) + i);
            *((uint64_t*)(start_addr_dest + 1204496) + i) = *((uint64_t*)(start_addr_source + 1204496) + i);
            *((uint64_t*)(start_addr_dest + 1604496) + i) = *((uint64_t*)(start_addr_source + 1604496) + i);
            *((uint64_t*)(start_addr_dest + 2004496) + i) = *((uint64_t*)(start_addr_source + 2004496) + i);
            *((uint64_t*)(start_addr_dest + 2404496) + i) = *((uint64_t*)(start_addr_source + 2404496) + i);
            *((uint64_t*)(start_addr_dest + 2804496) + i) = *((uint64_t*)(start_addr_source + 2804496) + i);
        }
        start_addr_dest += stride * sizeof(uint64_t);
        start_addr_source += stride * sizeof(uint64_t);
    }
    uint64_t cycle_end = csr_read(CSR_MCYCLE);
    uint64_t instr_end = csr_read(CSR_MINSTRET);
    printf("sequential memcopy muti stream hand write: range 0x%x bytes ends\n", range);
    printf("IPC in memcopy: %.5f\n", (float)(instr_end - instr_start) / (cycle_end - cycle_start));
}

void stride_test(uint64_t source, uint64_t dest, uint64_t range, uint64_t stride) {
    uint64_t start_addr_dest = dest;
    uint64_t end_addr_dest = start_addr_dest + range;
    uint64_t start_addr_source = source;
    printf("sequential stride test: range 0x%x bytes stride 0x%x bytes starts\n", range, stride);
    uint64_t cycle_start = csr_read(CSR_MCYCLE);
    uint64_t instr_start = csr_read(CSR_MINSTRET);
    while(start_addr_dest < end_addr_dest) {
        *((uint64_t*)(start_addr_dest)) = *((uint64_t*)(start_addr_source));
        start_addr_dest += stride;
        start_addr_source += stride;
    }
    uint64_t cycle_end = csr_read(CSR_MCYCLE);
    uint64_t instr_end = csr_read(CSR_MINSTRET);
    printf("sequential stride test: range 0x%x bytes stride 0x%x bytes ends\n", range, stride);
    printf("IPC in stride: %.5f\n", (float)(instr_end - instr_start) / (cycle_end - cycle_start));
}

void complex_stride_test(uint64_t source, uint64_t dest, uint64_t range) {
    uint64_t start_addr_dest = dest;
    uint64_t end_addr_dest = start_addr_dest + range;
    uint64_t start_addr_source = source;
    printf("complex stride test: range 0x%x bytes starts\n", range);
    uint64_t cycle_start = csr_read(CSR_MCYCLE);
    uint64_t instr_start = csr_read(CSR_MINSTRET);
    int i = 0;
    while(start_addr_dest < end_addr_dest) {
        *((uint64_t*)(start_addr_dest)) = *((uint64_t*)(start_addr_source));
        if(((i ++) % 3) == 2) {
            start_addr_dest += 0x7a0;
            start_addr_source += 0x7a0;
        }else {
            start_addr_dest += 0x30;
            start_addr_source += 0x30;
        }
    }
    uint64_t cycle_end = csr_read(CSR_MCYCLE);
    uint64_t instr_end = csr_read(CSR_MINSTRET);
    printf("complex stride test: range 0x%x bytes ends\n", range);
    printf("IPC in stride: %.5f\n", (float)(instr_end - instr_start) / (cycle_end - cycle_start));
}

uint64_t fib(uint64_t num) {
    if(num <= 2) {
        return 1;
    }else {
        return fib(num - 1) + fib(num - 2);
    }
}

void massive_cal(uint64_t ratio) {
    // do computing jobs
    // typical fetch, calulating, writeback sequence
    // printf("massive_cal\n");
    for(uint64_t i = 0; i < MAXRANGE; i += 8) {
        uint64_t data = *((uint64_t*)(PHYADDRSTART + i));
        data = data + fib(ratio);
        *((uint64_t*)(PHYADDRSTART + i)) = data;
    }
}

void bench_srand(uint64_t _seed) {
  seed = _seed & 0x7fff;
}

uint64_t bench_rand() {
  seed = (seed * (uint64_t)214013L + (uint64_t)2531011L);
  return (seed >> 16) & 0x7fff;
}

int main()
{
    const uint64_t memset_ratio = 100;
    const uint64_t memcopy_ratio = 0;
    const uint64_t is_stride = 0;
    const uint64_t decr = 0;
    const uint64_t muti_stream = 0;
    const uint64_t hand_write = 0;
    uint64_t i = 0;
    bench_srand(122);
    while(i++ < PATTENNUM) {
        uint64_t num = bench_rand() % 100;
        if(num < memset_ratio) {
            spb_memset(PHYADDRSTART, MAXRANGE, 0);
        }else if(num < memcopy_ratio){
            if(is_stride)
                stride_test(PHYADDRSTART, PHYADDRSTART + MAXRANGE, MAXRANGE, BLOCKOFFSET * 10);
                // complex_stride_test(PHYADDRSTART, PHYADDRSTART + MAXRANGE, MAXRANGE);
            else if(muti_stream)
                if(hand_write)
                    spb_memcopy_muti_stream_hand_write(PHYADDRSTART, PHYADDRSTART + MAXRANGE, MAXRANGE);
                else
                    spb_memcopy_muti_stream(PHYADDRSTART, PHYADDRSTART + MAXRANGE, MAXRANGE, BLOCKOFFSET * 1562500);
            else if(decr)
                spb_memcopy_decr(PHYADDRSTART, PHYADDRSTART + MAXRANGE, MAXRANGE);
            else
                spb_memcopy(PHYADDRSTART, PHYADDRSTART + MAXRANGE, MAXRANGE);
        }else {
            massive_cal(10);
        }
    }
    return 0;
}
