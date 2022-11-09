#include <klib.h>
#include <csr.h>

__attribute__((aligned(256)))
void ficom_loop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:" 
#if  CHOOSEO  ==0       
            "fadd.d  fs6,fs4,fs5;"
#elif CHOOSEO ==1
            "fmul.d  fs6,fs4,fs5;"
#elif CHOOSEO ==2
            "fdiv.d  fs6,fs5,fs4;"
#endif

#if CHOOSET ==0
            "fcvt.l.d s8,fs6;"
#elif CHOOSET ==1  
            "fmv.x.d  s8,fs6; "
#endif

#if CHOOSER ==0
            "fcvt.d.l fs5,s8;"
#elif CHOOSER == 1
            "fmv.d.x  fs5,s8;"
#endif

/*#if CHOOSEF ==0
            "fcvt.l.d s6,fs5;"
#elif CHOOSEF ==1
            "fmv.x.d  s6,fs5;"
#endif

#if CHOOSEI ==0
            "fcvt.d.l fs5,s6;"
#elif CHOOSEI == 1
            "fmv.d.x  fs5,s6; "
#endif
*/
            "addi s4 , s4 , 1;"
            "bleu s4,s5,loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            "li   s6 , 0x80000000;"
            "li   s7 , 0x80000000;"
            "li   s8 , 1;"
            "sd   s7 , 0(s6);"
            "li   t6 , 1;"
            "fcvt.d.l fs5,t6;"
            "fcvt.d.l fs4,s8;"
            "li   t5 ,0x80000000;"

            "csrr  s9 , mcycle;"
            "csrr  s10, minstret;"

            "jal   zero, loop;"

        "term:"
            "csrr s11 , mcycle;"
            "csrr t3  , minstret;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6","cc"

    );

}
int main(int argc,char* argv[]){

    unsigned long long busy_instr[10];
    unsigned long long busy_cycle[10];
    int i;
    //int j;
    printf("ifcom test\n");
    for( i=0;i<10;i++){
        ficom_loop(&busy_instr[i], &busy_cycle[i]);
    }
    for(i=0;i<10;i++){
        printf("i = %d;busy_instr = %d;busy_cycle = %8d\n",i,busy_instr[i],busy_cycle[i]);
    }
    for(i=0;i<10;i++)
        printf("the test is finish\n");


    return 0;
}