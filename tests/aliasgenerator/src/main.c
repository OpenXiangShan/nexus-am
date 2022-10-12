/* This test is used for random pressure test for hardware alias mechanism
 * usage: make ARCH=riscv64-xs
 */

#include <aliasgenerator.h>

#define RANGE_LEN(start, len) RANGE((start), (start + len))
#define PAGESIZE 0x1000 // 4KB
#define PAGE_TABLE_START_ADDR 0xc0000000UL // page table starts from here
#define TEST_DATA_START_ADDR 0x81000000UL // blank data pages start from here
#define TEST_INSTR_START_ADDR 0x82000000UL // instruction pages start from here

#define VIRTUAL_DATA_PAGE_START_ADDR PAGESIZE
#define VIRTUAL_DATA_PAGE_NUMBER 16

#define VIRTUAL_INSTR_PAGE_START_ADDR (VIRTUAL_DATA_PAGE_START_ADDR + VIRTUAL_DATA_PAGE_NUMBER * PAGESIZE)
#define VIRTUAL_INSTR_PAGE_NUMBER 16


#define RET_INSTRUCTION_CODE 0x8082

#define FENCE_INSTRUCTION_PROBABILITY 1000 // 1/1000
#define LONGFETCH_PROBABILITY 10 // 1/10

static _Area segments[] = {      // Kernel memory mappings
  RANGE_LEN(0x80000000, 0x1000000), // PMEM
  RANGE_LEN(0x40600000, 0x1000),    // uart
  RANGE_LEN(PAGE_TABLE_START_ADDR, 0x100000),  // page tables
  RANGE_LEN(TEST_DATA_START_ADDR, 0x100000),  // test datas
  RANGE_LEN(TEST_INSTR_START_ADDR, 0x100000),  // test instructions
  RANGE_LEN(0xa2000000, 0x10000) // clint
};

// meta data of a page, stored at the start of it's page
struct meta_data {
  volatile uint64_t paddr;
  volatile uint64_t ref_counter;
  volatile uint16_t instruction_ret;
};

uint64_t ref_counters[VIRTUAL_DATA_PAGE_NUMBER / 4] = {0};

// virtual page address starts from vp_start
uint64_t* vp_start = (uint64_t*)VIRTUAL_DATA_PAGE_START_ADDR;
// total virtual page numbers
uint64_t vp_number = (uint64_t)VIRTUAL_DATA_PAGE_NUMBER;
// physical page address starts from fp_start
uint64_t* fp_start = (uint64_t*)TEST_DATA_START_ADDR;

// virtual page address of instruction pages full of `ret` , which is adjacent to virtual data pages
uint64_t* instr_page_start = (uint64_t*)VIRTUAL_INSTR_PAGE_START_ADDR;
// total virtual instruction page numbers
uint64_t instr_page_number = (uint64_t)VIRTUAL_INSTR_PAGE_NUMBER;

extern _AddressSpace kas;

// map vp0 -> fp0; vp1 -> fp0; vp2 -> fp0; vp3 -> fp0; 
// map vp4 -> fp1; vp5 -> fp1; vp6 -> fp1; vp7 -> fp1; 
// ...

// map instr_page0 -> TEST_INSTR_START_ADDR; instr_page1 -> TEST_INSTR_START_ADDR; instr_page2 -> TEST_INSTR_START_ADDR; instr_page3 -> TEST_INSTR_START_ADDR; 
// map instr_page4 -> TEST_INSTR_START_ADDR + PAGESIZE; instr_page5 -> TEST_INSTR_START_ADDR + PAGESIZE; instr_page6 -> TEST_INSTR_START_ADDR + PAGESIZE; instr_page7 -> TEST_INSTR_START_ADDR + PAGESIZE; 
// ...

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

static inline void* get_random_access_addr() {
  return (void*)vp_start + (rand() % vp_number) * PAGESIZE;
}

static inline void* get_paddr(void* vaddr) {
  return (void*)fp_start + (vaddr - (void*)vp_start) / PAGESIZE / 4 * PAGESIZE;
}

static inline void* get_random_jal_addr() {
  return (void*)instr_page_start + (rand() % instr_page_number) * PAGESIZE;
}

static inline void icache_disturb() {
  void(*p)(void);
  if(rand() % 2 == 0) {
    p = get_random_jal_addr();
    printf(".");
  }else {
    p = (void*)&(((struct meta_data*)get_random_access_addr())->instruction_ret);
    printf("+");
  }
  // printf("jump to %p\n", p);
  (*p)();
}

static inline void issue_fence_instruction() {
  if((rand() % 2) == 0) {
    asm volatile("sfence.vma");
  }else {
    if((rand() % 2) == 0) {
      asm volatile("fence");
    }else {
      asm volatile("fence.i");
    }
  }
}

static inline void random_address_access() {
  void* vaddr = get_random_access_addr();
  void* paddr = get_paddr(vaddr);

  int idx = (vaddr - (void*)vp_start) / PAGESIZE / 4;

  // printf("access %p, paddr %p\n", vaddr, paddr);

  if( ((struct meta_data*)vaddr)->paddr != (uint64_t)paddr || ((struct meta_data*)vaddr)->ref_counter != (uint64_t)ref_counters[idx] ) {
    printf("error: data missmatch vaddr: %p paddr: %p, ref_counter: %p, got data: %p, got ref_counter: %p\n", vaddr, paddr, ref_counters[idx], ((struct meta_data*)vaddr)->paddr, ((struct meta_data*)vaddr)->ref_counter);
    _halt(1);
  }
  ((struct meta_data*)vaddr)->ref_counter++;
  ref_counters[idx]++;
}

void random_test(uint32_t seed) {
  uint64_t test_time = 0;

  srand(seed);
  while(1) {

    if((rand() % FENCE_INSTRUCTION_PROBABILITY) == 0) {
      printf("-");
      issue_fence_instruction();
    }else {
      if((rand() % LONGFETCH_PROBABILITY) == 0) {
        icache_disturb();
      }else {
        random_address_access();
      }
    }

    test_time++;
    if((test_time % 1000) == 0) {
      printf("*");
    }
  }
}

void trap_handler(void) {
  uint64_t scause;
  asm volatile("csrr %0, scause" : "=r"(scause));
  printf("panic: should not trigger any exception scause: %x\n", scause);
  _halt(1);
}

void set_alias_env() {
  // make page tables
  _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, sizeof(segments) / sizeof(segments[0]));
  // do map in data pages
  for(int i = 0; i < vp_number; i++) {
    void* vaddr = (void*)((uint64_t)vp_start + i * PAGESIZE);
    void* paddr = (void*)((uint64_t)fp_start + (i / 4) * PAGESIZE);
    printf("map %p -> %p\n", vaddr, paddr);
    _map(&kas, vaddr, paddr, PTE_W | PTE_R | PTE_A | PTE_D | PTE_X);
  }
  // do map in instruction pages
  for(int i = 0; i < instr_page_number; i++) {
    void* vaddr = (void*)((uint64_t)instr_page_start + i * PAGESIZE);
    void* paddr = (void*)((uint64_t)TEST_INSTR_START_ADDR + (i / 4) * PAGESIZE);
    printf("map %p -> %p\n", vaddr, paddr);
    _map(&kas, vaddr, paddr, PTE_W | PTE_R | PTE_A | PTE_D | PTE_X);
  }

  asm volatile("sfence.vma");
}

void init() {
  // set pmp so that not trigger access fault
  init_pmp();
  // delegate all to S
  asm volatile("csrw mideleg, %0" : : "r"(0xffff));
  asm volatile("csrw medeleg, %0" : : "r"(0xfffb));
  // initialize exception entry
  asm volatile("csrw stvec, %0" : : "r"(trap_handler));

  // fill in meta data in data pages
  for(void* pdata = segments[3].start; pdata < segments[3].end; pdata += PAGESIZE) {
    ((struct meta_data*)pdata)->paddr = (uint64_t) pdata;
    ((struct meta_data*)pdata)->ref_counter = 0;
    ((struct meta_data*)pdata)->instruction_ret = (uint16_t)RET_INSTRUCTION_CODE;
  }
  // fill in `ret` instruction in instruction pages
  for(uint16_t* pinstr = segments[4].start; pinstr < (uint16_t*)segments[4].end; pinstr += 1) {
    *pinstr = (uint16_t)RET_INSTRUCTION_CODE;
  }

  // enter S-mode
  uintptr_t status = MSTATUS_SPP(MODE_S);
  extern char _here;
  asm volatile(
    "csrw sstatus, %0;"
    "csrw sepc, %1;"
    "sret;"
    "_here:"
    : : "r"(status), "r"(&_here));
}


// just for debug
void print_page_tables(void* pagebase) {
  for(void* pgbase = pagebase; pgbase < pagebase + PAGESIZE; pgbase += 8) {
    PTE * pte = (PTE*)pgbase;
    if(*pte & PTE_V) {
      if(((*pte) & (PTE_R | PTE_W | PTE_X)) == 0) {
        print_page_tables((void*)PTE_ADDR(*pte));
      }else {
      printf("paddr: %p\n", PTE_ADDR(*pte));
      }
    }
  }
}

int main() {
    init();
    printf("init down\n");
    set_alias_env();
    printf("env set and start random test\n");
    // print_page_tables((void*)kas.ptr);
    random_test(1888);
    _halt(0);
    return 0;
}