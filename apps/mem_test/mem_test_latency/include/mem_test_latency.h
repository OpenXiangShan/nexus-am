#include <klib.h>
#include <csr.h>

#define BYTE (1)
#define KB (1024*BYTE)
#define MB (1024*KB)
#define GB (1024*MB)

#define _PERF_TEST_ADDR_BASE 0x80010000
//#define _PERF_TEST_ADDR_BASE 0x100010000
//#define _PERF_TEST_ADDR_BASE 0x80050000
#define _PERF_CACHELINE_SIZE_BYTE (8 * BYTE)
//#define _PERF_L1_SIZE_BYTE (32 * KB)
//#define _PERF_L1_SIZE_BYTE (33 * KB)
#define _PERF_L1_SIZE_BYTE (64 * KB)
#define _PERF_L2_SIZE_BYTE (1 * MB)
#define _PERF_L3_SIZE_BYTE (6 * MB)
#define _PERF_MEM_SIZE_BYTE (32 *MB)
#define _TEST_NUM_SIZE (32 * KB)
#define _STEP_SIZE (1 *MB)

uint32_t lfsr_next(uint32_t last) {
  uint32_t lfsr = last;
  lfsr ^= lfsr >> 7;
  lfsr ^= lfsr << 9;
  lfsr ^= lfsr >> 13;
  return lfsr;
}

uint64_t random_num_to_addr(uint64_t base_addr, uint32_t num, uint32_t size_limit) {
  uint32_t size_mask = size_limit - 1;
  uint32_t cache_line_mask = _PERF_CACHELINE_SIZE_BYTE - 1;
  uint32_t masked_num = num & (size_mask & (~cache_line_mask));
  return base_addr + masked_num;
}

void pseudo_random_warmup(uint64_t base_addr, uint32_t seed, unsigned blk_count, uint32_t size_limit) {
  unsigned count = 0;
  uint64_t cur_addr = base_addr;
  /*uint32_t init_seed = seed;*/
  while (count < blk_count) {
    uint32_t next_number = lfsr_next(seed);
    /*if (next_number == init_seed) {*/
    /*  printf("Loop detected, stop warmup\n");*/
    /*  assert(0);*/
    /*}*/
    uint64_t next_addr = random_num_to_addr(base_addr, next_number, size_limit);
    // assert(next_addr >= base_addr && next_addr < base_addr + size_limit);
    *((uint64_t*)cur_addr) = next_addr;

    count++;
    cur_addr = next_addr;
    seed = next_number;
  }
  printf("Pseudo random warmup %u cache blocks\n", blk_count);
}

void full_cache_warmup_i(uint64_t base_addr,uint64_t end_addr,uint64_t step,int choose){
    uint64_t num = 0;
    uint32_t cur_addr;
    assert(step % 8 ==0);
    assert(step >= 8);
    for(cur_addr = base_addr ; cur_addr < end_addr;){
        uint32_t next_addr = cur_addr + step;
        //choose ==0 test l1, next_address=address
        if (choose ==0) {
            *((uint64_t*)(uint64_t)cur_addr) = cur_addr;
        } else {
        //test l2/l3,next_address= address +64, visit next block
            *((uint64_t*)(uint64_t)cur_addr) = next_addr;
            /*printf("cur_addr: %x -> next_addr: %x\n",cur_addr,next_addr);*/
        }
        cur_addr = next_addr;
        num ++;
    }
    printf("Final address: %x\n", cur_addr);
}

__attribute__((aligned(256)))
void cache_iloop(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:" 
            "ld    s8,0(s6);"
            "ld    s6,0(s8);"

            "addi s4 , s4 , 1;"
            "bleu s4,s5,loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            "li   s6 , 0x80010000;"
            "li   s7 , 0;"
            "li   s8 , 0;"


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
