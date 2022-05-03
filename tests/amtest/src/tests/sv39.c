#include <amtest.h>

/*
 * RISC-V 64 SV39 Virutal Memory test
 */

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
static char *sv39_alloc_base = (char *)(0xc0000000UL);
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
static char *sv39_alloc_base = (char *)(0x2040000000UL);
#else
  // invalid arch
#endif

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
  _map(&kas, (void *)0x2100000000UL, (void *)0x2000020000, PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0x2200000000UL, (void *)0x2000020000, PTE_W | PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0x2300000000UL, (void *)0x2000020000, PTE_A | PTE_D);
  printf("memory map done\n");
  char *w_ptr = (char *)(0x2100000000UL);
  char *r_ptr = (char *)(0x2200000000UL);
  char *fault_ptr = (char *)(0x2300000000UL);
#else
  // invalid arch
#endif
  *w_ptr = 'a';
  printf("sv39 data written\n");
  assert(*r_ptr == 'a');
  printf("triggering fault\n");
  *fault_ptr = 'b';
  printf("should not reach here!\n");
}
