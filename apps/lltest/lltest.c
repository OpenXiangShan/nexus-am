#include <klib.h>
#include <csr.h>

#define xstr(s) str(s)
#define str(s)  #s
#define OP1 72057594037927935
#define OP2 4294967295
#define EMPTY 0
#define FULL 1


#if EMPTY == 1
__attribute__((aligned(256)))
void empty_load_load(unsigned long long* instr_count, unsigned long long* cycle_count ){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, _llinit;"
            //"xor s8  , zero , zero;"

        "_llloop:"
            "ld   s8 ,0(s7);"
            "fence;"
            "addi s4 , s4 , 1;"
            "bleu s4 , s5 , _llloop;"
            "jal      zero, _llterm;"

        "_llinit:"
            "mv   s4 , zero;"
            "li   s5 , 100;"
            "li   s6 , 0x80000000;"
            "li   s7 , 0x80000008;"
            "li   s8 , 0x8000000c;"
            "sd   s7 , 0(s6);"
            "sd   s8 , 0(s7);"

            "csrr s9 , mcycle ;"
            "csrr s10, minstret;"

            "jal  zero, _llloop;"

        "_llterm:"
            "csrr s11, mcycle ;"
            "csrr t3 , minstret;"
            "fence;"

            "subw %[c], s11, s9;"
            "subw %[i], t3 , s10;"

        : [c] "=r" (*cycle_count), [i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","cc"

    );
}
#endif
#if FULL == 1
__attribute__((aligned(256)))
void load_load_loop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, llinit;"
            //"xor s8  , zero , zero;"

        "llloop:"
            //xstr(INST)  " s8,  s6,  s7;"
            "ld   s7 ,0(s6);"
           // "fence;"

            "ld   s8 ,0(s7);"

            "fence;"
            "addi s4 , s4 , 1 ;"
            "bleu s4 , s5 , llloop;"

            "jal  zero ,llterm;"

        "llinit:"
            "mv   s4 , zero;"
            "li   s5 , 100;"
            "li   s6 , 0x80000000;"
            "li   s7 , 0x80000008;"
            "li   s8 , 0x8000000c;"
            "sd   s7 , 0(s6);"
            "sd   s8 , 0(s7);"


            "csrr  s9 , mcycle;"
            "csrr  s10, minstret;"

            "jal   zero, llloop;"

        "llterm:"
            "csrr s11 , mcycle;"
            "csrr t3  , minstret;"
            "fence;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","cc"

    );
}
#endif
/*__attribute__((aligned(256)))
void ll_loop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, llinit;"
            //"xor s8  , zero , zero;"

        "llloop:"
            //xstr(INST)  " s8,  s6,  s7;"
            "csrr  s9 , mcycle;"
            "csrr  s10, minstret;"

            "ld   s7 ,0(s6);"
            "csrr t4 ,mcycle;"
            "csrr t5 ,minstret;"

            "ld   s8 ,0(s7);"

            "fence;"
            "addi s4 , s4 , 1 ;"
            "subw t4 , t4 , s9;"
            "subw t5 , t5 , s10;"
            "add  s11, s11, t4;"
            "add  t3 , t3 , t5;"
            "bleu s4 , s5 , llloop;"

            "jal  zero ,llterm;"

        "llinit:"
            "mv   s4 , zero;"
            "li   s5 , 100;"
            "li   s6 , 0x80000000;"
            "li   s7 , 0x80000008;"
            "li   s8 , 0x8000000c;"
            "li   s11, 0;"
            "li    t3, 0;"
            "sd   s7 , 0(s6);"
            "sd   s8 , 0(s7);"


            //"csrr  s9 , mcycle;"
            //"csrr  s10, minstret;"

            "jal   zero, llloop;"

        "llterm:"
            //"csrr s11 , mcycle;"
            //"csrr t3  , minstret;"
            "fence;"

            //"subw  %[c], s11 , s9;"
            //"subw  %[i], t3  , s10;"
            "mv %[c] ,s11;"
            "mv %[i] ,t3;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","cc"

    );

}*/
int main(int argc,char* argv[]){

    unsigned long long ll_busy_cycles = 0, ll_busy_instrs = 0;
    #if FULL == 1
    load_load_loop(&ll_busy_instrs, &ll_busy_cycles);
    #endif

    unsigned long long ll_empty_cycles = 0, ll_empty_instrs = 0;
    
    #if EMPTY == 1
    empty_load_load(&ll_empty_instrs, &ll_empty_cycles);
    #endif

    unsigned long ll_instrs = ll_busy_instrs - ll_empty_instrs;
    unsigned long ll_cycles = ll_busy_cycles - ll_empty_cycles;

    printf("ll_busy_cycles %d\n",ll_busy_cycles);
    printf("ll_empty_cycles %d\n",ll_empty_cycles);

    printf("load to load instrs\t%d\tcycles\t%d\n", ll_instrs, ll_cycles);     

    //unsigned long long ll_busy_cycles = 0, ll_busy_instrs = 0;
    //load_load_loop(&ll_busy_instrs, &ll_busy_cycles);

   /* unsigned long long ll_cycles = 0, ll_instrs = 0;
    
    ll_loop(&ll_cycles, &ll_instrs);

//    unsigned long ll_instrs = ll_busy_instrs - ll_empty_instrs;
  //  unsigned long ll_cycles = ll_busy_cycles - ll_empty_cycles;

//    printf("ll_busy_cycles %d\n",ll_busy_cycles);
  //  printf("ll_empty_cycles %d\n",ll_empty_cycles);

    printf("load to load instrs\t%x\tcycles\t%x\n", ll_instrs, ll_cycles);   */  








   


    return 0;
}
