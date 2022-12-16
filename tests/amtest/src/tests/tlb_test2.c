#include <amtest.h>
#include <csr.h>
#include <riscv.h>

#define RANGE_LEN(start, len) RANGE((start), (start + len))

#define PAGESIZE 0x1000 // 4KB
#define _PERF_TEST_ADDR_BASE 0x80040000
#define PAGE_TABLE_START_ADDR 0xc0000000UL // page table starts from here
#define TEST_DATA_START_ADDR 0x83000000UL // blank data pages start from here
#define TEST_INSTR_START_ADDR 0x84000000UL // instruction pages start from here

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

/*__attribute__((aligned(256)))
void tlb_hit_test2(unsigned long long* instr_count, unsigned long long* cycle_count){
    *instr_count = 0;
    *cycle_count = 0;
    printf("1111\n");
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

}*/

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
            "li a7,-1;"
            "ecall;"

//            "li  s9 , 10;"
//            "li  s10, 10;"


 //           "csrr  s9 , mcycle;"
 //           "csrr  s10, minstret;"

            "jal   zero, loop;"
        "term:"
            "li a7,-1;"
            "ecall;"

            "li s11 , 11;"
            "li t3  , 11;"
//            "csrr s11 , mcycle;"
//            "csrr t3  , minstret;"

            "subw  %[c], s11 , s9;"
            "subw  %[i], t3  , s10;"

        : [c] "=r" (*cycle_count),[i] "=r" (*instr_count)
        : 
        : "zero","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6","cc"

    );

}
/*void tlb_hit_test2(uint64_t base_addr,int log){
  volatile int i_all = 0;
  volatile int i = 0;
  volatile uint64_t addr_now = base_addr;
  volatile uint64_t read_num;
  volatile uint64_t next_step_c = 64;
  volatile uint64_t next_step_t = 0x8000;
  _yield();
  for(i_all;i_all<500;i_all++){
    read_num = *((uint64_t*)addr_now);
    if(i == 63){
      next_step_c = 64;
      i=0;
    }
    addr_now = base_addr + next_step_c + next_step_t;
  //  printf("addr_now %lx\n",addr_now);
    next_step_c = next_step_c + 64;
    next_step_t = next_step_t + 0x8000;
    //printf("")

    

  }
  _yield();
  if(log ==3){
    printf("%d\n",read_num);
  }


}*/
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
    _vme_init(sv39_pgalloc2, sv39_pgfree2);
    printf("finish\n");

    uint64_t tlb_start_addr;
    uint64_t tlb_end_addr;
    uint64_t map_now = 0x900040000UL;
    uint64_t map_end = 0x900040000UL + 0x2000000;

    tlb_start_addr = map_now;//
    tlb_end_addr =  tlb_start_addr+ 600*8*PAGESIZE;//
    uint64_t paddr = 0x80040000;
    int i=0;
    int i_p = 0;

    printf("111\n");

   for(; map_now < map_end ; map_now +=PAGESIZE){   
    // printf("map_now %lx paddr%lx\n",map_now,paddr);
      _map(&kas,(void *)map_now,(void *)((uint64_t)(paddr )),PTE_W | PTE_R | PTE_A | PTE_D | PTE_X);
      i++;
      i_p++;
      if(i_p == 63 *8){
        i_p = 0;
        paddr = paddr + PAGESIZE;
      }
       
   }
    asm volatile("sfence.vma");
  /*  printf("******************************start_yield\n");
    _yield();
    printf("*******************************read from addr\n");
    printf("*********************************read_load 2\n");
    _yield();
    printf("*********************************test_end\n");  */  

    //full_tlb(tlb_start_addr,tlb_end_addr,PAGESIZE,2);
    tlb_hit_test(&instrs,&cycles);
   // tlb_hit_test2(0x900040040UL,1);
    printf("%d %d\n",tlb_num_2[0],tlb_num_2[1]);

}