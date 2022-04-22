#include <amtest.h>

void (*entry)() = NULL; // mp entry

static const char *tests[256] = {
  ['h'] = "hello",
  ['H'] = "display this help message",
  ['i'] = "interrupt/yield test",
  ['d'] = "scan devices",
  ['m'] = "multiprocessor test",
  ['t'] = "real-time clock test",
  ['k'] = "readkey test",
  ['v'] = "display test",
  ['a'] = "audio test",
  ['p'] = "x86 virtual memory test",
};

#define PMP_3

void pmp_test() {
  printf("start pmp test\n");
  #ifdef PMP_1
  volatile int *a = (int *)(0x90000040UL);
  *a = 1; // should trigger a fault
  #endif
  #ifdef PMP_2
  int *b = (int *)(0xa0000000UL);
  *b = 1; // should not trigger a fault
  #endif
  #ifdef PMP_3
  int *c = (int *)(0xb00000040UL);
  *c = 1; // should trigger a fault
  #endif
  asm volatile("ebreak");
}


static char *sv39_alloc_base = (char *)(0xc0000000UL);
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
  _map(&kas, (void *)0x900000000UL, (void *)0x80020000, PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0xa00000000UL, (void *)0x80020000, PTE_W | PTE_R | PTE_A | PTE_D);
  _map(&kas, (void *)0xb00000000UL, (void *)0x80020000, PTE_A | PTE_D);
  printf("memory map done\n");
  char *w_ptr = (char *)(0xa00000000UL);
  char *r_ptr = (char *)(0x900000000UL);
  char *fault_ptr = (char *)(0xb00000000UL);
  *w_ptr = 'a';
  printf("sv39 data written\n");
  assert(*r_ptr == 'a');
  printf("triggering fault\n");
  *fault_ptr = 'b';
  printf("should not reach here!\n");
}

int main(const char *args) {
  char arg = 's';
  switch (arg) {
    CASE('h', hello);
    CASE('i', hello_intr, IOE, CTE(simple_trap), REEH(simple_trap), RCEH(simple_trap));
    CASE('e', external_intr, IOE, CTE(external_trap), REEH(external_trap), RTEH(external_trap));
    CASE('d', devscan, IOE);
    CASE('m', finalize, PRE_MPE(args[1]), MPE(mp_print));
    CASE('t', rtc_test, IOE);
    CASE('k', keyboard_test, IOE);
    CASE('v', video_test, IOE);
    CASE('a', audio_test, IOE);
    CASE('p', vm_test, CTE(vm_handler), VME(simple_pgalloc, simple_pgfree));
    CASE('c', pmp_test, CTE(simple_trap));
    CASE('s', sv39_test, IOE, CTE(simple_trap));
    case 'H':
    default:
      printf("Usage: make run mainargs=*\n");
      for (int ch = 0; ch < 256; ch++) {
        if (tests[ch]) {
          printf("  %c: %s\n", ch, tests[ch]);
        }
      }
  }
  return 1;
}
