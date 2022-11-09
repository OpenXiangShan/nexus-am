#include <klib.h>
#include <csr.h>

__attribute__((aligned(256)))
void fcom_loop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:"            
/*#if   CHOOSEO == 0
            "fadd.d  fs6,fs4,fs5;"//fs6 = fs4(1)+fs5(1)=2
#elif CHOOSEO == 1
            "fmul.d  fs6,fs4,fs5;"//fs6 =1
#elif CHOOSEO == 2*/
            "fdiv.d  fs6,fs4,fs5;"//fs6 = fs4(1)/fs5(1)=1
//#endif

//#if   CHOOSET ==0
            //"fadd.d fs7,fs6,fs4;"//fs7 = fs6(1)+fs4(1) = 2
            "fadd.d  fs7,fs6,fs8;"//fs7 = fs6(1)+fs4(1) = 2
/*#elif CHOOSET ==1
            "fmul.d fs7,fs6,fs4;"//fs7 = fs6()
#elif CHOOSET ==2
            "fdiv.d fs7,fs6,fs4;"
#endif*/

/*#if   CHOOSER ==0
            "fadd.d fs5,fs7,fs4;"
#elif CHOOSER ==1
            "fmul.d fs5,fs7,fs4;"
#elif CHOOSER ==2*/
            "fdiv.d  fs5,fs7,fs4;"
//#endif

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
            "fcvt.d.l fs8,s4;"
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
    printf("fcom test\n");
    for( i=0;i<10;i++){
        fcom_loop(&busy_instr[i], &busy_cycle[i]);
    }
    for(i=0;i<10;i++){
        printf("i = %d;busy_instr = %d;busy_cycle = %6d\n",i,busy_instr[i],busy_cycle[i]);
    }
    for(i=0;i<10;i++)
        printf("the test is finish\n");


    return 0;
}