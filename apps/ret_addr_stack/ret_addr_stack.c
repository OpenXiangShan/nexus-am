#include <klib.h>
#include <csr.h>

#define NUM_ITER        1E6

void f(int n)
{
        if (n > 1)
                f(n - 1);
        return;
}

/*
 * Return Address Stack (RAS) Capacity Test:
 *
 * The RAS is a stack-like structure used to predict function return addresses.
 * When calling a function (bl/blr), the processor pushes the return address onto
 * the RAS. When returning from a function (ret), the processor pops from the RAS
 * and uses this as the predicted target address for the return.
 *
 * This test aims to measure the number of entries in the RAS.
 *
 * Suppose the RAS has N entries. If we execute a chain of k nested function calls,
 * each call pushes its return address to the RAS. When k > N, k - N addresses will
 * not fit into the RAS and will overwrite older entries. This causes those earlier
 * function return addresses to be predicted incorrectly which slows down program
 * execution. By measuring at which value of k program execution starts to slow down,
 * we can estimate the value of N.
 */
int main()
{
        int depth;
        int i;
        uint64_t cycle_1;
        uint64_t cycle_2;
        uint64_t inst_1;
        uint64_t inst_2;
        double cycle;
        double inst;
        //clock_t start, end;
        //double time;

        printf("title: Return Address Stack Capacity\n");
        printf("x_scale: linear\n");
        printf("y_scale: linear\n");

        for (depth = 64; depth >= 1; depth--) {
               // start = clock();
                cycle_1 = csr_read(CSR_MCYCLE);
                inst_1  = csr_read(CSR_MINSTRET);
                for (i = 0; i < NUM_ITER; i++)
                        f(depth);
                cycle_2 = csr_read(CSR_MCYCLE);
                inst_2  = csr_read(CSR_MINSTRET);
                //end = clock();
                cycle = cycle_2 - cycle_1;
                inst  = inst_2 - inst_1;


                printf("call depth: %d, cycle: %lf, inst: %lf\n",depth, cycle, inst);
                printf("call depth: %d, cycle per bench: %lf, inst per bench: %lf\n",depth, cycle / depth, inst /depth);
        }

        return 0;
}