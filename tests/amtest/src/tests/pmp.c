#include <amtest.h>
#include <csr.h>
#include <xsextra.h>

/*
 * Note that to be able to run this test, PMP should be set up before AM jump to supervisor
 * You may find related initialzation code in __am_init_cte64()
 */

#define EXCEPTION_LOAD_ACCESS_FAULT 5
#define EXCEPTION_STORE_ACCESS_FAULT 7

inline int inst_is_compressed(uint64_t addr){
  uint8_t byte = *(uint8_t*)addr;
  return (byte & 0x3) != 0x3; 
}

volatile uint64_t pmp_store_access_fault_to_be_reported = 0;
volatile uint64_t pmp_store_access_fault_reported = 0;
volatile uint64_t pmp_load_access_fault_to_be_reported = 0;
volatile uint64_t pmp_load_access_fault_reported = 0;

volatile int result_blackhole = 0;

void reset_result_flags() {
  pmp_store_access_fault_to_be_reported = 0;
  pmp_store_access_fault_reported = 0;
  pmp_load_access_fault_to_be_reported = 0;
  pmp_load_access_fault_reported = 0;
}

void result_check() {
  assert(!(pmp_store_access_fault_to_be_reported && pmp_load_access_fault_to_be_reported));
  if (pmp_store_access_fault_to_be_reported) {
    if (!pmp_store_access_fault_reported || pmp_load_access_fault_reported) {
      printf("pmp_store_access_fault_reported %x, pmp_load_access_fault_reported %x\n", 
        pmp_store_access_fault_reported, pmp_load_access_fault_reported);
      _halt(1);
    }
  } else if (pmp_load_access_fault_to_be_reported) {
    if (!pmp_load_access_fault_reported || pmp_store_access_fault_reported) {
      printf("pmp_store_access_fault_reported %x, pmp_load_access_fault_reported %x\n", 
        pmp_store_access_fault_reported, pmp_load_access_fault_reported);
      _halt(1);
    }
  } else {
    if (pmp_load_access_fault_reported || pmp_store_access_fault_reported) {
      printf("pmp_store_access_fault_reported %x, pmp_load_access_fault_reported %x\n", 
        pmp_store_access_fault_reported, pmp_load_access_fault_reported);
      _halt(1);
    }
  }
  // result check passed, reset flags
  pmp_store_access_fault_to_be_reported = 0;
  pmp_store_access_fault_reported = 0;
  pmp_load_access_fault_to_be_reported = 0;
  pmp_load_access_fault_reported = 0;
}

_Context* pmp_store_access_fault_handler(_Event* ev, _Context *c) {
  printf("store access fault triggered, sepc %lx\n", c->sepc);
  pmp_store_access_fault_reported = 1;
  // skip the inst that triggered the exception
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4; 
  // printf("goto %x\n", c->sepc);
  return c;
}

_Context* pmp_load_access_fault_handler(_Event* ev, _Context *c) {
  printf("load access fault triggered, sepc %lx\n", c->sepc);
  pmp_load_access_fault_reported = 1;
  // skip the inst that triggered the exception
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4; 
  // printf("goto %x\n", c->sepc);
  return c;
}

void pmp_test() {
  irq_handler_reg(EXCEPTION_STORE_ACCESS_FAULT, &pmp_store_access_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_ACCESS_FAULT, &pmp_load_access_fault_handler);
  printf("start pmp test\n");
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
  // Case: store to address protected by pmp
  pmp_store_access_fault_to_be_reported = 1;
  volatile int *a = (int *)(0x90000040UL);
  *a = 1; // should trigger a fault
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: store to normal cacheable address
  int *b = (int *)(0xa0000000UL);
  *b = 1; // should not trigger a fault
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: store to address protected by pmp tor
  pmp_store_access_fault_to_be_reported = 1;
  int *c = (int *)(0xb0000040UL);
  *c = 1; // should trigger a fault
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: load from address protected by pmp
  pmp_load_access_fault_to_be_reported = 1;
  volatile int *d = (int *)(0x90000040UL);
  result_blackhole = (*d); // should trigger a fault
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: load from address protected by pmp tor
  pmp_load_access_fault_to_be_reported = 1;
  volatile int *e = (int *)(0xb0000040UL);
  result_blackhole = (*e); // should trigger a fault
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: store to address protected by pmp (use pmpcfg2)
  pmp_store_access_fault_to_be_reported = 1;
  int *f = (int *)(0xb0010000UL);
  *f = 1; // should trigger a fault
  result_check();
  printf("line %d passed\n", __LINE__);
  
  // Case: lr from address protected by pmp
  pmp_load_access_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xb0000040;"
    "lr.d s5, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: sc to address protected by pmp
  pmp_store_access_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xb0000040;"
    "sc.d s5, s5, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: amo to address protected by pmp
  pmp_store_access_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xb0000040;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: amo to address protected by pmp (w,!r)
  pmp_store_access_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xb0008000;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check();
  printf("line %d passed\n", __LINE__);

  // Case: amo to address protected by pmp (!w,r)
  pmp_store_access_fault_to_be_reported = 1;
  asm volatile(
    "li s4, 0xb0004000;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check();
  printf("line %d passed\n", __LINE__);

#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  // TODO: update pmp test for southlake
  pmp_store_access_fault_to_be_reported = 0;
  int *b = (int *)(0x2030000000UL);
  *b = 1; // should not trigger a fault
  result_check();

  pmp_store_access_fault_to_be_reported = 1;
  volatile int *a = (int *)(0x2010000040UL);
  *a = 1; // should trigger a fault
  result_check();
#else
  // invalid arch
  printf("invalid arch\n");
  _halt(1);
#endif
}