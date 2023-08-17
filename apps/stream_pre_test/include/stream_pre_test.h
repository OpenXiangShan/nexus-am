#include <klib.h>
#include <csr.h>

#define BYTE (1)
#define KB (1024*BYTE)
#define MB (1024*KB)
#define GB (1024*MB)

#define _LAST_ADDR_BYTE  (160*MB)

#define _PERF_TEST_ADDR_BASE 0x80000000
#define _PERF_TEST_ADDR_BASE_1 0x80010000
__attribute__((aligned(256)))
void stream_pre(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:" 
            "ld    s6,0(s0);"
            "ld    s7,72(s0);"
            "ld    s8,144(s0);"
            "ld    t4,216(s0);"
            "ld    t5,288(s0);"

            "addi s4 , s4 , 1;"
            "addi s0 ,s0,320;"
            "bleu s4,s5,loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 524288;"
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

uint64_t stream_pre_address(uint64_t step,uint64_t roundstep,uint64_t last_addr){
    //uint64_t num =0;
    assert(step %8 ==0);
    assert(step >=8);
    uint64_t cur_addr;
    uint64_t num0 =0;
    uint64_t num1 =0;
    uint64_t num2 =0;
    uint64_t num3 =0;
    uint64_t num4 =0;
    uint64_t num_all =0;

    for (cur_addr = _PERF_TEST_ADDR_BASE;cur_addr<last_addr;){
        num0 = (*(uint64_t*)cur_addr);
        num1 = (*(uint64_t*)(cur_addr+step));
        num2 = (*(uint64_t*)(cur_addr+2*step));
        num3 = (*(uint64_t*)(cur_addr+3*step));
        num4 = (*(uint64_t*)(cur_addr+4*step));
        cur_addr = cur_addr +roundstep;
        num_all= num_all +num0+num1+num2+num3+num4;
    }
    return num_all;
}

