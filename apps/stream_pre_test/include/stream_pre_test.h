#include <klib.h>
#include <csr.h>

#define BYTE (1)
#define KB (1024*BYTE)
#define MB (1024*KB)
#define GB (1024*MB)

#define _PERF_TEST_ADDR_BASE 0x80000000
__attribute__((aligned(256)))
void stream_pre(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:" 
            "ld    s6,0(s0);"
            "ld    s7,8(s0);"
            "ld    s8,16(s0);"
            "ld    t4,24(s0);"
            "ld    t5,32(s0);"

            "addi s4 , s4 , 1;"
            "addi s0 ,s0,64;"
            "bleu s4,s5,loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            "li   s6 , 0;"
            "li   s7 , 0;"
            "li   s8 , 0;"
            "li   t4 , 0;"
            "li   t5 , 0;"
            "li   s0 , 0x80000000;"


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

