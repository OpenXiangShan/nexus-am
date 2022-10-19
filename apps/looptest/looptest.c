#include <klib.h>
#include <csr.h>

__attribute__((aligned(256)))
void load_use_loop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
   // *finish_flag = 1;
    asm volatile(
        //    "mv   ra, zero;"
            "jal  zero, init_lp;"

        "loop_lp:"
            "addi s4 ,s4  ,0;"

            "sd   s4 ,0(s8);"//M[s8]=s4
            "ld   s4 ,0(s8);"//s4 = M[s8] =0
//            "addi s8 ,s8   ,1;"
            "addi s4 , s4 , 1 ;"
            "bleu s4 , s5 , loop_lp;"

            "jal  zero ,term_lp;"

        "init_lp:"
            "li   s4 , 0;"
            "li   s5 , 10;"
            "li   s6 , 0x80000000;"
           // "li   s7 , 0x80000008;"
            "li   s8 , 0x8000000c;"
           // "sd   s7 , 0(s6);"
           // "sd   s8 , 0(s7);"

            "csrr s9 ,mcycle;"
            "csrr s10,minstret;"

            "jal   zero, loop_lp;"

        "term_lp:"
            "csrr s11 , mcycle;"
            "csrr t3  , minstret;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"
           // "subw %[c], %[c2], %[c1];"
            //"subw %[i], %[i2] , %[i1];"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t0","ra","cc"

    );

}


int main(){
    unsigned long long busy_cycles[10],busy_instrs[10];
    printf("1\n");

    for(int i = 0;i<10;i++){
        load_use_loop(&busy_instrs[i], &busy_cycles[i]);
        printf("i = %d\n",i);
    }
        

    for(int j = 0;j<10;j++){
        printf("j = %d busy_instrs %d,busy_cycles %d\n",j,busy_instrs[j],busy_cycles[j]);

        printf("\n");
    }

    return 0;


}