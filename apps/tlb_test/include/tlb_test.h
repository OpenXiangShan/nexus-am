#include <am.h>
#include <xsextra.h>
#include <klib.h>
#include <riscv.h>
#include <klib-macros.h>
#include <nemu.h>

#define RANGE_LEN(start, len) RANGE((start), (start + len))

#define PAGESIZE 0x1000 // 4KB
#define PAGE_TABLE_START_ADDR 0xc0000000UL // page table starts from here
#define TEST_DATA_START_ADDR 0x81000000UL // blank data pages start from here
#define TEST_INSTR_START_ADDR 0x82000000UL // instruction pages start from here

static _Area segments[] = {      // Kernel memory mappings
  RANGE_LEN(0x80000000, 0x1000000), // PMEM
  RANGE_LEN(0x40600000, 0x1000),    // uart
  RANGE_LEN(PAGE_TABLE_START_ADDR, 0x100000),  // page tables
  RANGE_LEN(TEST_DATA_START_ADDR, 0x100000),  // test datas
  RANGE_LEN(TEST_INSTR_START_ADDR, 0x100000),  // test instructions
  RANGE_LEN(0xa2000000, 0x10000) // clint
};

static char *sv39_alloc_base = (char *)(PAGE_TABLE_START_ADDR);

static uintptr_t sv39_alloced_size = 0;
void* sv39_pgalloc(size_t pg_size) {
  assert(pg_size == 0x1000);
  // printf("sv39 pgalloc called\n");
  void *ret = (void *)(sv39_alloc_base + sv39_alloced_size);
  sv39_alloced_size += pg_size;
  return ret;
}

void sv39_pgfree(void *ptr) {
  return ;
}

__attribute__((aligned(256)))
void tlb_hit_test(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init;"
            "xor s8  , zero , zero;"

        "loop:" 
            "ld    s8,0(s6);"
            "sub   s7,s8,s8;"
    //        "ld    s6,0(s8);"
            "addi  s8,s7,64;"
            "add   s6,s6,s8;"

            "addi s4 , s4 , 1;"
            "bleu s4,s5,loop;"

            "jal  zero ,term;"

        "init:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            //"li   s6 , 0x80000000;"
            "li   s6 , 0x80040000;"
            "li   s7 , 0x80040000;"
            "li   s7 , 0;"
            "li   s8 , 0;"
            "ld   s8 ,0(s6);"
            "addi s6,s6,8;"


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