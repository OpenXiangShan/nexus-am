#include <amtest.h>
#include <pmp.h>
#include <xsextra.h>

/*
 * RISC-V 64 SV39 Virutal Memory test
 */

#define EXCEPTION_LOAD_PAGE_FAULT 13
#define EXCEPTION_STORE_PAGE_FAULT 15

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
static char *sv39_alloc_base = (char *)(0xc0000000UL);
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
static char *sv39_alloc_base = (char *)(0x2040000000UL);
#else
  // invalid arch
#endif

uint64_t page_fault_to_be_reported = 0;

inline int inst_is_compressed(uint64_t addr){
  uint8_t byte = *(uint8_t*)addr;
  return (byte & 0x3) != 0x3; 
}

_Context* store_page_fault_handler(_Event* ev, _Context *c) {
  printf("store page fault triggered\n");
  if(!page_fault_to_be_reported){
    _halt(1); // something went wrong
  }
  page_fault_to_be_reported = 0;
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4;
  return c;
}

_Context* load_page_fault_handler(_Event* ev, _Context *c) {
  printf("load page fault triggered\n");
  if(!page_fault_to_be_reported){
    _halt(1); // something went wrong
  }
  page_fault_to_be_reported = 0;
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4;
  return c;
}

static uintptr_t sv39_alloced_size = 0;
void* sv39_pgalloc(size_t pg_size) {
  assert(pg_size == 0x1000);
  printf("sv39 pgalloc called\n");
  void *ret = (void *)(sv39_alloc_base + sv39_alloced_size);
  sv39_alloced_size += pg_size;
  return ret;
}

void sv39_pgfree(void *ptr) {
  return ;
}

extern _AddressSpace kas;
#include <riscv.h>

void sv39_test() {
  printf("start sv39 test\n");
  _vme_init(sv39_pgalloc, sv39_pgfree);
  printf("sv39 setup done\n");
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
  _map(&kas, (void *)0x900000000UL, (void *)0x80020000, PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0xa00000000UL, (void *)0x80020000, PTE_W | PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0xb00000000UL, (void *)0x80020000, PTE_A | PTE_D);
  printf("memory map done\n");
  char *w_ptr = (char *)(0xa00000000UL);
  char *r_ptr = (char *)(0x900000000UL);
  char *fault_ptr = (char *)(0xb00000000UL);
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  _map(&kas, (void *)0x2100000000UL, (void *)0x2000020000, PTE_W | PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0x2200000000UL, (void *)0x2000020000, PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0x2300000000UL, (void *)0x2000020000, PTE_A | PTE_D);
  printf("memory map done\n");
  char *w_ptr = (char *)(0x2100000000UL);
  char *r_ptr = (char *)(0x2200000000UL);
  char *fault_ptr = (char *)(0x2300000000UL);
#else
  // invalid arch
  _halt(1);
#endif
  irq_handler_reg(EXCEPTION_STORE_PAGE_FAULT, &store_page_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_PAGE_FAULT, &load_page_fault_handler);

  printf("test sv39 data write\n");
  *w_ptr = 'a';

  printf("test sv39 data read\n");
  assert(*r_ptr == 'a');

  printf("test sv39 store page fault\n");
  page_fault_to_be_reported = 1;
  *fault_ptr = 'b'; // store: not compressed 
  if(page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 load page fault\n");
  page_fault_to_be_reported = 1;
  *w_ptr = *fault_ptr;
  if(page_fault_to_be_reported){
    _halt(1);
  }
  _halt(0);
}
