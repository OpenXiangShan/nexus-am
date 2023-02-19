#include <amtest.h>
#include <csr.h>
#include <xsextra.h>

/*
 * RISC-V 64 SV39 Virutal Memory test
 */

#define EXCEPTION_LOAD_ACCESS_FAULT 5
#define EXCEPTION_STORE_ACCESS_FAULT 7
#define EXCEPTION_LOAD_PAGE_FAULT 13
#define EXCEPTION_STORE_PAGE_FAULT 15

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
static char *sv39_alloc_base = (char *)(0xc0000000UL);
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
static char *sv39_alloc_base = (char *)(0x2040000000UL);
#else
  // invalid arch
#endif

volatile uint64_t load_page_fault_to_be_reported = 0;
volatile uint64_t store_page_fault_to_be_reported = 0;
volatile uint64_t load_access_fault_to_be_reported = 0;
volatile uint64_t store_access_fault_to_be_reported = 0;


inline int inst_is_compressed(uint64_t addr){
  uint8_t byte = *(uint8_t*)addr;
  return (byte & 0x3) != 0x3;
}

_Context* store_page_fault_handler(_Event* ev, _Context *c) {
  printf("store page fault triggered\n");
  if(!store_page_fault_to_be_reported){
    _halt(1); // something went wrong
  }
  store_page_fault_to_be_reported = 0;
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4;
  return c;
}

_Context* load_page_fault_handler(_Event* ev, _Context *c) {
  printf("load page fault triggered\n");
  if(!load_page_fault_to_be_reported){
    _halt(1); // something went wrong
  }
  load_page_fault_to_be_reported = 0;
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4;
  return c;
}

_Context* load_access_fault_handler(_Event* ev, _Context *c) {
  printf("load access fault triggered\n");
  if(!load_access_fault_to_be_reported){
    _halt(1); // something went wrong
  }
  load_access_fault_to_be_reported = 0;
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4;
  return c;
}

_Context* store_access_fault_handler(_Event* ev, _Context *c) {
  printf("store access fault triggered\n");
  if(!store_access_fault_to_be_reported){
    _halt(1); // something went wrong
  }
  store_access_fault_to_be_reported = 0;
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
  asm volatile("sfence.vma");
  printf("test sv39 data write\n");
  *w_ptr = 'a';

  printf("test sv39 data read\n");
  assert(*r_ptr == 'a');

  printf("test sv39 store page fault\n");
  store_page_fault_to_be_reported = 1;
  *fault_ptr = 'b';
  if(store_page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 load page fault\n");
  load_page_fault_to_be_reported = 1;
  *w_ptr = *fault_ptr;
  if(load_page_fault_to_be_reported){
    _halt(1);
  }
  _halt(0);
}

/*
 * RISC-V 64 SV39 raise access fault when high 20 bits of ppn is not zero
 */

void sv39_ppn_af_test() {
  printf("start sv39 test\n");
  _vme_init(sv39_pgalloc, sv39_pgfree);
  printf("sv39 setup done\n");
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
  _map(&kas, (void *)0x900000000UL, (void *)0x80020000, PTE_W | PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0xa00000000UL, (void *)0x80020000, PTE_W | PTE_R | PTE_A | PTE_D);
  uint64_t addr = 0xb00000000UL;
  for (int i = 0; i < 100; i++) {
    _map_fault(&kas, (void *)(addr + 0x1000 * i), (void *)0x80020000, PTE_W | PTE_R | PTE_A | PTE_D);
  }
  printf("memory map done\n");
  char *w_ptr = (char *)(0xa00000000UL);
  char *r_ptr = (char *)(0x900000000UL);
  char *fault_ptr[100] = {0};
  for (int i = 0; i < 100; i++) {
    fault_ptr[i] = (char *)(addr + 0x1000 * i);
  }
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  _map(&kas, (void *)0x2100000000UL, (void *)0x2000020000, PTE_W | PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0x2200000000UL, (void *)0x2000020000, PTE_W | PTE_R | PTE_A | PTE_D);
  uint64_t addr = 0x2300000000UL;
  for (int i = 0; i < 100; i++) {
    _map_fault(&kas, (void *)(addr + 0x1000 * i), (void *)0x2000020000, PTE_W | PTE_R | PTE_A | PTE_D);
  }
  printf("memory map done\n");
  char *w_ptr = (char *)(0x2100000000UL);
  char *r_ptr = (char *)(0x2200000000UL);
  char *fault_ptr[100] = {0};
  for (int i = 0; i < 100; i++) {
    fault_ptr[i] = (char *)(addr + 0x1000 * i);
  }}
#else
  // invalid arch
  _halt(1);
#endif
  irq_handler_reg(EXCEPTION_STORE_ACCESS_FAULT, &store_access_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_ACCESS_FAULT, &load_access_fault_handler);
  asm volatile("sfence.vma");
  printf("test sv39 data write\n");
  *w_ptr = 'a';

  printf("test sv39 data read\n");
  assert(*r_ptr == 'a');

  for (int i = 0; i < 100; i++) {
    printf("test sv39 store access fault %d\n", i);
    store_access_fault_to_be_reported = 1;
    *fault_ptr[i] = 'b';
    if(store_access_fault_to_be_reported){
      _halt(1);
    }

    printf("test sv39 load access fault %d\n", i);
    load_access_fault_to_be_reported = 1;
    *w_ptr = *fault_ptr[i];
    if(load_access_fault_to_be_reported){
      _halt(1);
    }
  }
  _halt(0);
}

/*
 * RISC-V 64 SV39 Hugepage + Hugepage Atom Inst test
 */

void sv39_hp_atom_test() {
  printf("start sv39 hugepage atom test\n");
  _vme_init(sv39_pgalloc, sv39_pgfree);
  printf("sv39 setup done\n");
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
  _map(&kas, (void *)0x900000000UL, (void *)0x80200000, PTE_R | PTE_A | PTE_D);
  // allocate a metapage, not protected by pmp
  _map_rv_hugepage(&kas, (void *)0xa00000000UL, (void *)0x80200000, PTE_W | PTE_R | PTE_A | PTE_D, 1);
  // allocate a metapage, not protected by pmp, without write perm
  _map_rv_hugepage(&kas, (void *)0xb00000000UL, (void *)0x80200000, PTE_R | PTE_A | PTE_D, 1);
  // allocate a metapage, not protected by pmp, without read perm
  _map_rv_hugepage(&kas, (void *)0xc00000000UL, (void *)0x80200000, PTE_W | PTE_A | PTE_D, 1);
  // allocate a metapage, protected by pmp (!rw)
  _map_rv_hugepage(&kas, (void *)0xd00000000UL, (void *)0xb0000000, PTE_W | PTE_R | PTE_A | PTE_D, 1);
  char *normal_rw_ptr = (char *)(0x900000000UL);
  char *hp_rw_ptr     = (char *)(0xa00000000UL);
  char *hp_r_ptr      = (char *)(0xb00000000UL);
  char *hp_w_ptr      = (char *)(0xc00000000UL);
  char *hp_pmp_ptr    = (char *)(0xd00000000UL);
  printf("memory map done\n");
#else
  // invalid arch
  char *normal_rw_ptr = (char *)(0x900000000UL);
  char *hp_rw_ptr     = (char *)(0xa00000000UL);
  char *hp_r_ptr      = (char *)(0xb00000000UL);
  char *hp_w_ptr      = (char *)(0xc00000000UL);
  char *hp_pmp_ptr    = (char *)(0xd00000000UL);
  _halt(1);
#endif
  irq_handler_reg(EXCEPTION_STORE_PAGE_FAULT, &store_page_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_PAGE_FAULT, &load_page_fault_handler);
  irq_handler_reg(EXCEPTION_STORE_ACCESS_FAULT, &store_access_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_ACCESS_FAULT, &load_access_fault_handler);

  asm volatile("sfence.vma");
  printf("test sv39 hugepage data write\n");
  *hp_rw_ptr = 'a';

  printf("test sv39 hugepage data read\n");
  assert(*hp_rw_ptr == 'a');

  printf("test sv39 normalpage data read\n");
  assert(*normal_rw_ptr == 'a');

  printf("test sv39 hugepage store page fault\n");
  store_page_fault_to_be_reported = 1;
  *hp_r_ptr = 'b';
  if(store_page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 hugepage load page fault\n");
  load_page_fault_to_be_reported = 1;
  *hp_rw_ptr = *hp_w_ptr;
  if(load_page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 hugepage store access fault\n");
  store_access_fault_to_be_reported = 1;
  *hp_pmp_ptr = 'b';
  if(store_page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 hugepage load access fault\n");
  load_access_fault_to_be_reported = 1;
  *hp_rw_ptr = *hp_pmp_ptr;
  if(load_access_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 hugepage atom store page fault\n");
  store_page_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xb00000000;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  if(store_page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 hugepage atom load page fault\n");
  load_page_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xc00000000;"
    "lr.d s5, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  if(load_page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 hugepage atom store access fault\n");
  store_access_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xd00000000;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  if(store_page_fault_to_be_reported){
    _halt(1);
  }

  printf("test sv39 hugepage atom load access fault\n");
  load_access_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xd00000000;"
    "lr.d s5, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  if(load_access_fault_to_be_reported){
    _halt(1);
  }

  _halt(0);
}