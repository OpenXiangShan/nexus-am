#include <klib.h>
#include <csr.h>

//#define EMPTY 1
//#define FULL 1

//#if EMPTY==1
__attribute__((aligned(256)))
void empty_load_use(unsigned long long* instr_count, unsigned long long* cycle_count ,unsigned long long* instr_count1, unsigned long long* cycle_count1,unsigned long long* instr_count2, unsigned long long* cycle_count2){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
          //  "mv  ra, zero;"
            "jal zero, _init;"
            //"bgtu t3, zero, _init;"
            //"xor s8  , zero , zero;"

        "_loop:"
            "ld   s8 ,(s6);"
            "addi s8 ,s8   ,1;"
            //"addi s8 , s6, 1;"
            "fence;"
            "addi s4 , s4 , 1;"
            "bleu s4 , s5 , _loop;"
            "jal      zero, _term;"

        "_init:"
            "li     s4   ,0;"
            "li     s5   ,100;"
            "li     s6   ,0x80000000;"//80000000
            "li     s7   ,0x80000008;"//80000008
            "li     t0   ,0x8000000c;"//8000000c
            "sd     s7 , 0(s6);"
            "sd     s8 , 0(s7);"
            "fence;"

            //"csrr   s9 , mcycle ;"
            "csrr   %[c1] , mcycle ;"
            //"csrr   s10, minstret;"
            "csrr   %[i1], minstret;"
            "fence;"

            "jal    zero, _loop;"

        "_term:"
            "fence;"
            "csrr %[c2], mcycle ;"
            //"csrr s11, mcycle ;"
            "csrr %[i2] , minstret;"
            //"csrr t3 , minstret;"
            "fence;"

            "subw %[c], %[c2], %[c1];"
            "subw %[i], %[i2] , %[i1];"
            "fence;"

        : [c] "=r" (*cycle_count), [i] "=r" (*instr_count),[c1] "=r" (*cycle_count1), [i1] "=r" (*instr_count1),[c2] "=r" (*cycle_count2), [i2] "=r" (*instr_count2)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","ra","t0","cc"

    );
}
//#endif
//#if FULL == 1
__attribute__((aligned(256)))
void load_use_loop(unsigned long long* instr_count, unsigned long long* cycle_count ,unsigned long long* instr_count1, unsigned long long* cycle_count1,unsigned long long* instr_count2, unsigned long long* cycle_count2){
    *instr_count = 0;
    *cycle_count = 0;
   // *finish_flag = 1;
    asm volatile(
        //    "mv   ra, zero;"
            "jal  zero, init;"

        "loop:"
            "ld   s8 ,(s6);"
            "addi s8 ,s8   ,1;"

            "fence;"
            "addi s4 , s4 , 1 ;"
            "bleu s4 , s5 , loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 100;"
            "li   s6 , 0x80000000;"
            "li   s7 , 0x80000008;"
            "li   s8 , 0x8000000c;"
            "sd   s7 , 0(s6);"
            "sd   s8 , 0(s7);"

            "fence;"
            "csrr  %[c1] , mcycle;"
            //"csrr  s9 , mcycle;"
            "csrr  %[i1], minstret;"
            //"csrr  s10, minstret;"
            "fence;"

            "jal   zero, loop;"

        "term:"
            "fence;"
            //"csrr s11 , mcycle;"
            "csrr %[c2] , mcycle;"
            //"csrr t3  , minstret;"
            "csrr %[i2]  , minstret;"
            "fence;"

            //"subw  %[c], s11 , s9;"
            //"subw  %[i], t3  , s10;"
            "subw %[c], %[c2], %[c1];"
            "subw %[i], %[i2] , %[i1];"
            "fence;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count),[c1] "=r" (*cycle_count1), [i1] "=r" (*instr_count1),[c2] "=r" (*cycle_count2), [i2] "=r" (*instr_count2)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t0","ra","cc"

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
    unsigned long long busy_cycles1[10],busy_instrs1[10];
    unsigned long long busy_cycles2[10],busy_instrs2[10];

    
//    #if FULL ==1
    for(int i = 0;i<10;i++){
        load_use_loop(&busy_instrs[i], &busy_cycles[i],&busy_instrs1[i], &busy_cycles1[i],&busy_instrs2[i], &busy_cycles2[i]);
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
        printf("j = %d busy_instrs1 %d,busy_cycles1 %d\n",j,busy_instrs1[j],busy_cycles1[j]);
        printf("j = %d busy_instrs2 %d,busy_cycles2 %d\n",j,busy_instrs2[j],busy_cycles2[j]);
        printf("\n");
    }
   // assert(0); 
    return 0;


}