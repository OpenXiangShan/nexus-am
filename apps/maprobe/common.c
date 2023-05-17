#include "maprobe.h"

struct perf perf;
uint64_t _perf_g_total_samples = 0;

void _perf_start_timer()
{
#ifndef PERF_SIM
    perf.instrcnt = csr_read(CSR_MINSTRET);
    perf.cycle = csr_read(CSR_MCYCLE);
#endif
}

void _perf_end_timer()
{
#ifndef PERF_SIM
    perf.cycle = csr_read(CSR_MCYCLE) - perf.cycle;
    perf.instrcnt = csr_read(CSR_MINSTRET) - perf.instrcnt;
#endif
}

void _perf_print_timer()
{
    printf("cycle %d inst %d ipc %lf\n", perf.cycle, perf.instrcnt, (float)perf.instrcnt/perf.cycle);
}

void _perf_calibrate()
{
#ifndef PERF_SIM
    // csr read delay
    uint64_t cycle_1 = csr_read(CSR_MCYCLE);
    uint64_t cycle_2 = csr_read(CSR_MCYCLE);
    perf.csr_read_cycle = cycle_2-cycle_1;
    printf("perf_calibrate: csr_read_cycle %d\n", perf.csr_read_cycle);

    // csr read inst cost
    uint64_t inst_1 = csr_read(CSR_MINSTRET);
    uint64_t inst_2 = csr_read(CSR_MINSTRET);
    perf.csr_read_ninst = inst_2-inst_1;
    printf("perf_calibrate: csr_read_ninst %d\n", perf.csr_read_ninst);
#else
    printf("running in simulation environment, hpm read disabled\n");
#endif
}

void _perf_blackhole(uint64_t value)
{
    *(uint64_t*) _PERF_BLACKHOLE = value;
}
