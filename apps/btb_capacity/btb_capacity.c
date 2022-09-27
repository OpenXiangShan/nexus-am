#include <klib.h>
#include <csr.h>
//#include <time.h>
//#include <stdio.h>


#define NUM_ITER   1E5

//void foo() {return;}

/*
 * Branch Target Buffer (BTB) Capacity Test:
 *
 * The BTB is a structure used to cache branch target addresses so that subsequent
 * instructions can be fetched as soon as possible. The BTB is organized as a
 * set-associative cache.
 *
 * This test aims to measure the number of entries in the BTB.
 *
 * Suppose the BTB has N entries. If we execute k branches at sequential addresses, we
 * will sequentially fill k entries in the BTB. When k > N, k - N branches will be
 * capacity misses and will evict older entries from the BTB. This causes those earlier
 * branches to miss in the BTB, resulting in front-end stalls which slows down program
 * execution. By measuring at which value of k program execution starts to slow down,
 * we can estimate the value of N.
 */




int main(){
    const int num_branches = 1;/* vary from 1..8192 */
    int i;
    uint64_t cycle_1;
    uint64_t cycle_2;
    uint64_t inst_1;
    uint64_t inst_2;
    double cycle;
    double inst;

    cycle_1 = csr_read(CSR_MCYCLE);
    inst_1  = csr_read(CSR_MINSTRET);

    for(i=0; i<NUM_ITER; i++){
        for(int j=0;j<num_branches;j++){
            asm volatile(
                    "jal zero,foo;"
                "foo:"
                    "nop;"     
                :::);
        }
        
    }

    cycle_2 = csr_read(CSR_MCYCLE);
    inst_2  = csr_read(CSR_MINSTRET);

    cycle = cycle_2 - cycle_1;
    inst  = inst_2 - inst_1;


    printf("number of branches: %d, cycle : %lf, inst: %lf\n",num_branches, cycle, inst);
    printf("number of branches: %d, cycle per bench : %lf, inst per bench : %lf\n",num_branches, cycle / num_branches , inst /num_branches);
    return 0;


}

