#include <klib.h>
#include <csr.h>

#define COUNT (15 * 192)
#define PERIET 1

void csrrw_loop(uint64_t loop_count);
int main(void) {
    printf("beg\n");
    uint64_t cycle_count = 0;
    uint64_t start_time = csr_read(CSR_MCYCLE);
    uint64_t start_inst = csr_read(CSR_MINSTRET);

    
    csrrw_loop(PERIET * COUNT);


    uint64_t end_time = csr_read(CSR_MCYCLE);
    uint64_t end_inst = csr_read(CSR_MINSTRET);
    cycle_count = end_time - start_time;
    uint64_t inst_count = end_inst - start_inst;
    printf("cycle count: %lu\n", cycle_count);
    printf("inst count: %lu\n", inst_count);
    printf("ipc: %lf\n", (float)inst_count/cycle_count);
    printf("end\n");
    return 0;
}