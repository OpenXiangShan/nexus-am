//#include <stdio.h>
#include <klib.h>
//#include "util.h"

#define xstr(s) str(s)
#define str(s)  #s
#define OP1 72057594037927935
#define OP2 4294967295



__attribute__((aligned(256)))
void empty_loop(unsigned long long* instr_count, unsigned long long* cycle_count ,unsigned long long a,unsigned long long b){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, _init;"
            "xor s8  , zero , zero;"

        "_loop:"
            "fence;"
            "addi s4 , s4 , 1;"
            "bleu s4 , s5 , _loop;"
            "jal      zero, _term;"

        "_init:"
            "mv   s4 , zero;"
            "li   s5 , 100;"
            "mv   s6 , %[a];"
            "mv   s7 , %[b];"

            "csrr s9 , mcycle ;"
            "csrr s10, minstret;"

            "jal  zero, _loop;"

        "_term:"
            "csrr s11, mcycle ;"
            "csrr t3 , minstret;"
            "fence;"

            "subw %[c], s11, s9;"
            "subw %[i], t3 , s10;"

        : [c] "=r" (*cycle_count), [i] "=r" (*instr_count)
        : [a]"r"(a),[b]"r"(b)
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","cc"

    );
}
__attribute__((aligned(256)))
void busy_loop(unsigned long long* instr_count, unsigned long long* cycle_count,unsigned long long a,unsigned long long b){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:"
            xstr(INST)  " s8,  s6,  s7;"
            "fence;"
            "addi s4 , s4 , 1 ;"
            "bleu s4 , s5 , loop;"

            "jal  zero ,term;"

        "init:"
            "mv   s4 , zero;"
            "li   s5 , 100;"
            "mv   s6 , %[a];"
            "mv   s7 , %[b];"

            "csrr  s9 , mcycle;"
            "csrr  s10, minstret;"

            "jal   zero, loop;"

        "term:"
            "csrr s11 , mcycle;"
            "csrr t3  , minstret;"
            "fence;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : [a]"r"(a),[b]"r"(b)
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","cc"

    );

}
int main(int argc,char* argv[]){
    unsigned long long op[8]={1,255,65535,16777215,4294967295,1099511627775,281474976710655,72057594037927935};
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            unsigned long long op1 = op[i];
            unsigned long long op2 = op[j];
            unsigned long long busy_cycles = 0, busy_instrs = 0;
            busy_loop(&busy_instrs, &busy_cycles,op1,op2);

            unsigned long long empty_cycles = 0, empty_instrs = 0;
    
            empty_loop(&empty_instrs, &empty_cycles,op1,op2);

            unsigned long instrs = busy_instrs - empty_instrs;
            unsigned long cycles = busy_cycles - empty_cycles;

            printf("busy_cycles %d\n",busy_cycles);
            printf("empty_cycles %d\n",empty_cycles);

            printf("op1\t%llx\top2\t%llx\tinstrs\t%x\tcycles\t%x\n", op1, op2, instrs, cycles);            
        }
    }


    return 0;
}
