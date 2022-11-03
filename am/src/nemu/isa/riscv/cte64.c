#include <riscv.h>
#include <nemu.h>
#include <printf.h>
extern void __am_timervec(void);

static void init_machine_exception() {
  // set M-mode exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_timervec));
}

int g_config_disable_timer = 0; // dirty hack of __am_init_cte64(), to be refactored
extern void init_timer();
extern void enable_timer();
extern void init_pmp(); 
extern void enable_pmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t lock, uint8_t permission);
extern void enable_pmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uint8_t permission);
#include <csr.h>

static void init_eip() {
  // enable machine external interrupt (mie.meip and mstatus.mie)
  asm volatile("csrs mie, %0" : : "r"(1 << 11));
  asm volatile("csrs mstatus, %0" : : "r"(1 << 3));
}

void __am_init_cte64() {
  // set delegation (do not deleg illegal instruction exception)
  asm volatile("csrw mideleg, %0" : : "r"(0xffff));
  asm volatile("csrw medeleg, %0" : : "r"(0xfffb));

  // set PMP to access all memory in S-mode
  // asm volatile("csrw pmpaddr8, %0" : : "r"(-1));
  // asm volatile("csrw pmpcfg2, %0" : : "r"(31));
  
  init_pmp();
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
  // protect 0x90000000 + 0x10000 for test purpose
  printf("enable_pmp\n");
  enable_pmp(1, 0x90000000, 0x10000, 0, 0); // !rw
  // printf("pmp NA inited\n");
  // protect 0xb00000000 + 0x100
  enable_pmp_TOR(3, 0xb0000000, 0x1000, 0, 0); // !rw
  //printf("pmp TOR inited\n");
  enable_pmp_TOR(5, 0xb0004000, 0x1000, 0, PMP_R); // r,!w
  enable_pmp_TOR(7, 0xb0008000, 0x1000, 0, PMP_W); // !r, w
  enable_pmp_TOR(9, 0xb0010000, 0x1000, 0, 0); // !r, w
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  // protect 0x210000000 + 0x10000 for test purpose
  enable_pmp(1, 0x2010000000, 0x10000, 0, 0);
  // printf("pmp NA inited\n");
  // protect 0x240000000 + 0x100
  enable_pmp_TOR(4, 0x2040000000, 0x100, 0, 0);
  //printf("pmp TOR inited\n");
#else
  // invalid arch
#endif

  init_machine_exception();
  init_timer();
  if(!g_config_disable_timer){
    enable_timer();
  }
  init_eip();

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
