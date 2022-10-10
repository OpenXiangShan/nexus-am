#include <klib.h>
#include <csr.h>

//#define EMPTY 1
//#define FULL 1

//#if EMPTY==1
__attribute__((aligned(256)))
void empty_load_use(unsigned long long* instr_count, unsigned long long* cycle_count ){
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

            "csrr   s9 , mcycle ;"
            "csrr   s10, minstret;"
            "fence;"

            "jal    zero, _loop;"

        "_term:"
            "fence;"
            "csrr s11, mcycle ;"
            "csrr t3 , minstret;"
            "fence;"

            "subw %[c], s11, s9;"
            "subw %[i], t3 , s10;"
            "fence;"

        : [c] "=r" (*cycle_count), [i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","ra","t0","cc"

    );
}
//#endif
//#if FULL == 1
__attribute__((aligned(256)))
void load_use_loop(unsigned long long* instr_count, unsigned long long* cycle_count ){
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
            "csrr  s9 , mcycle;"
            "csrr  s10, minstret;"
            "fence;"

            "jal   zero, loop;"

        "term:"
            "fence;"
            "csrr s11 , mcycle;"
            "csrr t3  , minstret;"
            "fence;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"
            "fence;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t0","ra","cc"

    );

}
//#endif

int main(){

    unsigned long long busy_cycles = 0, busy_instrs = 0;
    unsigned long long empty_cycles = 0, empty_instrs = 0;
//    #if FULL ==1
    load_use_loop(&busy_instrs, &busy_cycles);
  //  #endif
    //#if EMPTY ==1
    empty_load_use(&empty_instrs, &empty_cycles);
    //#endif
    unsigned long instrs = busy_instrs - empty_instrs;
    unsigned long cycles = busy_cycles - empty_cycles;

    printf("busy_cycles %d\n",busy_cycles);
    printf("empty_cycles %d\n",empty_cycles);

    printf("load to use instrs\t%d\tcycles\t%d\n", instrs, cycles);  
   // assert(0); 
    return 0;


}