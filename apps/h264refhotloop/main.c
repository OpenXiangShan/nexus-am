#include <klib.h>
#include <csr.h>

#define COUNT (8 * 192)
#define PERIET 100
void h264refhotloop(uint64_t source, uint64_t dest, uint32_t counter);
void h264refhotloop_vec(uint64_t source, uint64_t dest, uint32_t counter);
int main(void) {
    printf("beg\n");
    // 12KB memcpy
    // h264refhotloop(0x88000000UL, 0x90000000UL, COUNT);
    // h264refhotloop(0x88000000UL, 0x90000000UL, COUNT);
    // h264refhotloop(0x88000000UL, 0x90000000UL, COUNT);
    // h264refhotloop(0x88000000UL, 0x90000000UL, COUNT);
    // h264refhotloop(0x88000000UL, 0x90000000UL, COUNT);
    h264refhotloop_vec(0x88000000UL, 0x90000000UL, COUNT);
    h264refhotloop_vec(0x88000000UL, 0x90000000UL, COUNT);
    h264refhotloop_vec(0x88000000UL, 0x90000000UL, COUNT);
    h264refhotloop_vec(0x88000000UL, 0x90000000UL, COUNT);
    uint64_t cycle_count = 0;
    uint64_t start_time = csr_read(CSR_MCYCLE);
    uint64_t start_inst = csr_read(CSR_MINSTRET);
    for (int i = 0; i < PERIET; i++) {
        // h264refhotloop(0x88000000UL, 0x90000000UL, COUNT);
        h264refhotloop_vec(0x88000000UL, 0x90000000UL, COUNT);
    }
    uint64_t end_time = csr_read(CSR_MCYCLE);
    uint64_t end_inst = csr_read(CSR_MINSTRET);
    cycle_count = end_time - start_time;
    uint64_t inst_count = end_inst - start_inst;
    printf("cycle count: %lu\n", cycle_count);
    printf("inst count: %lu\n", inst_count);
    printf("ipc: %lf\n", (float)inst_count/cycle_count);
    printf("bandwidth: %lfB/Cycle\n", (float)COUNT*8*PERIET/cycle_count);
    printf("end\n");
    return 0;
}