#include <klib.h>
#include <csr.h>

//#define EMPTY 1
//#define FULL 1

//#if EMPTY==1

//#endif
//#if FULL == 1
__attribute__((aligned(256)))
void load_use_loop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
   // *finish_flag = 1;
    asm volatile(
        //    "mv   ra, zero;"
            "jal  zero, init;"

        "loop:"
            "sub  t4 , s4 ,s4;"
            "add  s8 , s8 ,t4;"
            //"add  s6 , s6 ,t4;"//busy
        //    "ld   s8 ,(s6);"//busy

            "addi s8 ,s8   ,0;"
            "add  s4,s4,s8;"

            "addi s4 , s4 , 1 ;"
            "bleu s4 , s5 , loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            "li   s6 , 0x80000000;"
            //"li   s7 , 0x80000008;"
            "li   s7 , 0;"
           // "li   s8 , 0x8000000c;"
            "li   s8 , 0;"
            "sd   s7 , 0(s6);"
         //   "sd   s8 , 0(s7);"

            //"csrr  %[c1] , mcycle;"
            
            //"csrr  %[i1], minstret;"
            "csrr  s9 , mcycle;"
            "csrr  s10, minstret;"

            "jal   zero, loop;"

        "term:"
            //"mv s11 , s4;"
            //"csrr %[c2] , mcycle;"
            //"mv t3  , s5;"
            //"csrr %[i2]  , minstret;"
            "csrr s11 , mcycle;"
            "csrr t3  , minstret;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"
           // "subw %[c], %[c2], %[c1];"
            //"subw %[i], %[i2] , %[i1];"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t0","t4","ra","cc"

    );

}
//#endif

int main(){

    /*unsigned long long busy_cycles = 0, busy_instrs = 0;
    unsigned long long busy_cycles1 = 0, busy_instrs1 = 0;
    unsigned long long busy_cycles2 = 0, busy_instrs2 = 0;
    unsigned long long empty_cycles = 0, empty_instrs = 0;
    unsigned long long empty_cycles1 = 0, empty_instrs1 = 0;
    unsigned long long empty_cycles2 = 0, empty_instrs2 = 0;*/
    unsigned long long busy_cycles[10],busy_instrs[10];
    //uint64_t lu_cycle;


    
//    #if FULL ==1
    for(int i = 0;i<10;i++){
        load_use_loop(&busy_instrs[i], &busy_cycles[i]);
    }
        
  //  #endif
    //#if EMPTY ==1
 //   empty_load_use(&empty_instrs, &empty_cycles,&empty_instrs1, &empty_cycles1,&empty_instrs2, &empty_cycles2);
    //#endif
  //  unsigned long instrs = busy_instrs - empty_instrs;
  //  unsigned long cycles = busy_cycles - empty_cycles;

   // printf("the result is this\n");

   // printf("busy_cycles1 %d,busy_cycles2 %d\n",busy_cycles1,busy_cycles2);
   // printf("empty_cycles1 %d,empty_cycles2 %d\n",empty_cycles1,empty_cycles2);

//    printf("load to use instrs\t%d\tcycles\t%d\n", instrs, cycles);  
    for(int j = 0;j<10;j++){
        printf("j = %d busy_instrs %d,busy_cycles %d\n",j,busy_instrs[j],busy_cycles[j]);

        printf("\n");
    }
    //lu_cycle = csr_read(CSR_MCYCLE);
    //printf("lu_cycle %d\n",lu_cycle);
   // assert(0);
   //assert(0); 
    return 0;


}