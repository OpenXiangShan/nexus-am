#include <amtest.h>
#include <csr.h>
#include <riscv.h>

#define RANGE_LEN(start, len) RANGE((start), (start + len))

const static uint64_t pmem_base = 0x80000000UL;

#define PAGESIZE 0x1000 // 4KB
#define TLB_TEST_ADDR_BASE (pmem_base + 0x40000UL)
#define PAGE_TABLE_START_ADDR (pmem_base + 0x40000000UL) // page table starts from here
#define TEST_DATA_START_ADDR (pmem_base + 0x3000000UL) // blank data pages start from here
#define TEST_INSTR_START_ADDR (pmem_base + 0x4000000UL) // instruction pages start from here

extern _AddressSpace kas;
static char *sv39_alloc_base_2 = (char *)(PAGE_TABLE_START_ADDR);

static uintptr_t sv39_alloced_size_2 = 0;
void* sv39_pgalloc2(size_t pg_size) {
  assert(pg_size == 0x1000);
  // printf("sv39 pgalloc called\n");
  void *ret = (void *)(sv39_alloc_base_2 + sv39_alloced_size_2);
  sv39_alloced_size_2 += pg_size;
  return ret;
}

void sv39_pgfree2(void *ptr) {
  return ;
}

uint64_t tlb_i_2 =0;
uint64_t tlb_num_2[10];
uint64_t tlb_instr_2[10];

_Context *external_trap2(_Event ev, _Context *ctx) {

  tlb_num_2[tlb_i_2]=csr_read(CSR_MCYCLE);
  tlb_i_2++;
  return ctx;
}

__attribute__((aligned(256)))
void tlb_hit_test2(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    asm volatile(
            "jal zero, init_2;"
            "xor s8  , zero , zero;"

        "loop_2:"
            "ld    s8,0(s6);"
            "sub   s7,s8,s8;"
    //        "ld    s6,0(s8);"
            //"addi  s8,s7,64;"
            "add  s8,s7,a2;"
            "add   s6,s6,s8;"

            "addi s4 , s4 , 1;"
            "bleu s4,s5,loop_2;"

            "jal  zero ,term_2;"

        "init_2:"
            "li   s4 , 0;"
            "li   s5 , 500;"
            //"li   s6 , 0x80000000;"
            //"li   s6 , 0x80040040;"
            //"li   s7 , 0x80040000;"
            "li   s6 , 0x900040080;"
            "li   s7 , 0x900040000;"
            "li   s7 , 0;"
            "li   s8 , 0;"
            "lui  a2 , 8;"
            //"ld   s8 ,0(s6);"
            //"addi s6,s6,8;"
            "li a7,-1;"
            "ecall;"

            "li  s9 , 10;"
            "li  s10, 10;"
            "jal   zero, loop_2;"
        "term_2:"
            "li a7,-1;"
            "ecall;"

            "li s11 , 11;"
            "li t3  , 11;"
            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        :
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6","cc"

    );

}

void full_tlb (uint64_t base_addr,uint64_t end_addr,uint64_t step,int level){
    volatile uint64_t num = 0;
    volatile uint64_t read_num2 =0;
    printf("full_tlb %lx\n",base_addr);
    for(uint64_t cur_addr = base_addr;cur_addr < end_addr;){
        uint64_t next_addr;
        if(level == 1){
            next_addr = cur_addr + step;
        }
        else{
            next_addr = cur_addr + 8*step;
        }
        read_num2 = *((uint64_t*)cur_addr);
        cur_addr = next_addr;
        num++;


    }
    if(level ==3){
        printf("%d\n",read_num2);
    }


}

void tlb_test2(){
    unsigned long long instrs,cycles;
    instrs =1;
    cycles = 1;
    _vme_init(sv39_pgalloc2, sv39_pgfree2);
//    printf("finish\n");
//    printf("%lx\n",0x80040000+500*0x1000);


    uint64_t map_now = 0x900040000UL;
    uint64_t map_end = 0x900040000UL + 0x2000000;

    uint64_t tlb_start_addr = map_now;//
    uint64_t tlb_end_addr =  tlb_start_addr+ 600*8*PAGESIZE;//
    uint64_t paddr = TLB_TEST_ADDR_BASE;

   for(; map_now < map_end ; map_now +=PAGESIZE){
     _map(&kas,(void *)map_now,(void *)((uint64_t)(paddr )),PTE_W | PTE_R | PTE_A | PTE_D | PTE_X);
     paddr += PAGESIZE;
   }
    asm volatile("sfence.vma");


    full_tlb(tlb_start_addr,tlb_end_addr,PAGESIZE,2);
    tlb_hit_test2(&instrs,&cycles);

    printf("the result is %d %d\n",tlb_num_2[0],tlb_num_2[1]);

}