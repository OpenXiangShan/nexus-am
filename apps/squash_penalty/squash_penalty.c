#include <klib.h>
#include <csr.h>
void foo( ){
 //   i=i+1;
}
int main(){
    int i;
    int num[10000];
    for (i=0;i<10000;i++){
        num[i] = rand()%10;
    }

    uint64_t cycle_1;
    uint64_t cycle_2;
    uint64_t inst_1;
    uint64_t inst_2;
    double cycle;
    double inst;

    cycle_1 = csr_read(CSR_MCYCLE);
    inst_1  = csr_read(CSR_MINSTRET);

    for(i = 0; i<10000;i++){
        if(num[i]<5) foo();   
    }
    cycle_2 = csr_read(CSR_MCYCLE);
    inst_2  = csr_read(CSR_MINSTRET);

    cycle   = cycle_2 - cycle_1;
    inst    = inst_2 - inst_1;
    printf("squash_penalty:cycle : %lf,inst : %lf\n",cycle,inst);
    printf("e_cycle:%lf,e_inst:%lf\n",cycle /5000,inst/5000);

}