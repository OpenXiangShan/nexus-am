#include <klib.h>
#include <csr.h>

#define NUM_ITER   1E5

//void foo() {return ;}

/*
 * Branch Target Buffer (BTB) Associativity Test:
 *
 * The BTB is a structure used to cache branch target addresses so that subsequent
 * instructions can be fetched as soon as possible. The BTB is organized as a
 * set-associative cache.
 *
 * This test aims to measure the associativity of the BTB.
 *
 * Suppose the BTB has S sets and W ways. If we execute a sequence of branches
 * at various stride distances from each other, we will fill different BTB entries.
 * When the stride is equal to S, all branches will map to the same set, and if we
 * execute enough branches, will cause conflict misses in the BTB which slows down
 * program execution. By measuring at which stride conflict misses first occur, we
 * can estimate the value of S and W.
 */

int main(){
    const int stride = 1;
    int i;
    int j;
    int k;
    uint64_t cycle_1;
    uint64_t cycle_2;
    uint64_t inst_1;
    uint64_t inst_2;
    double cycle;
    double inst;
    //printf("11111111111111111111111\n");

    cycle_1 = csr_read(CSR_MCYCLE);
    inst_1  = csr_read(CSR_MINSTRET);
    for(i=0; i< NUM_ITER;i++){
   // for(i=0; i< 10;i++){
        /* update inner .rept to match stride - 1 */
        for(j =0;j < 64;j++){
        //for(j =0;j < 10;j++){
            //printf("finish j\n");
            asm volatile(
                    "jal zero,foo;"
                "foo:"
                    "nop;"     
                :::);
            for(k =0;k < stride-1;k++){
                asm("nop \n");
              //  printf("finish k\n");
            }
                

        }
    }
    cycle_2 = csr_read(CSR_MCYCLE);
    inst_2  = csr_read(CSR_MINSTRET);

    cycle = cycle_2 - cycle_1;
    inst  = inst_2 - inst_1;

    printf("stride: %d, cycle: %lf ,inst:%lf\n", stride, cycle, inst);
    return 0;
  



}