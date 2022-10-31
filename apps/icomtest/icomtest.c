#include <klib.h>
#include <csr.h>

#define xstr(s) str(s)
#define str(s)  #s

//#define CHOOSEO  0
//#define CHOOSET  1

__attribute__((aligned(256)))
void icom_loop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:"
            "sub  t4, s4, s4;"//t4 =0
            "add  s6, s6, t4;"//s6=80000000
            //xstr(INST)  " s8,  s6,  s7;"
#if   CHOOSEO == 0
            "add  s8,s6,t4;"//s8 =8000000
#elif CHOOSEO == 1
            "mul  s8,s6,t6;"
#elif CHOOSEO == 2
            "div  s8,s6,t6;"
#elif CHOOSEO == 3
            "ld   s8,0(s6);"
#endif

#if   CHOOSET ==0
            "add t5,s8,t4;"
#elif CHOOSET ==1
            "mul t5,s8,t6;"
#elif CHOOSET ==2
            "div t5,s8,t6;"
#elif CHOOSET ==3
            "sd s8 ,0(s8);"
#elif CHOOSET ==4
            "ld t5 ,0(s8);"

#endif
            "ld s8,0(t5);"
//            "div  s8, s6, s7;"//full
            "sub  t4 ,s8, s8;"//full
            //"sub  t4 ,s6,s6;"//empty
            "add  s4 ,s4, t4;"
            "addi s4 , s4 , 1;"
            //[c]
            //"%[d]"
            "bleu s4,s5,loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            "li   s6 , 0x80000000;"
            //"li   s7 , 0x80000008;"
            "li   s7 , 0x80000000;"
           // "li   s8 , 0x8000000c;"
            "li   s8 , 1;"
            "sd   s7 , 0(s6);"
            "li   t6 , 1;"
            "li   t5 ,0x80000000;"
            //""
         //   "sd   s8 , 0(s7);"

            //"csrr  %[c1] , mcycle;"
            
            //"csrr  %[i1], minstret;"
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
    printf("icom test\n");
    for( i=0;i<10;i++){
        icom_loop(&busy_instr[i], &busy_cycle[i]);
    }
    for(i=0;i<10;i++){
        printf("i = %d;busy_instr = %d;busy_cycle = %d\n",i,busy_instr[i],busy_cycle[i]);
    }
    for(i=0;i<10;i++)
        printf("the test is finish\n");


    return 0;
}