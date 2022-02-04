#include <riscv.h>
#include <nemu.h>

#ifdef __ARCH_RISCV64_NOOP
#define CLINT_MMIO (RTC_ADDR - 0xbff8)
#define TIME_INC 0x80000
#else
#define CLINT_MMIO 0xa2000000
#define TIME_INC 0x800
#endif

#define CLINT_MTIMECMP (CLINT_MMIO + 0x4000)

extern void __am_timervec(void);

void __am_init_cte64() {
  // set delegation (do not deleg illegal instruction exception)
  asm volatile("csrw mideleg, %0" : : "r"(0xffff));
  asm volatile("csrw medeleg, %0" : : "r"(0xfffb));

  // set PMP to access all memory in S-mode
  asm volatile("csrw pmpaddr0, %0" : : "r"(-1));
  asm volatile("csrw pmpcfg0, %0" : : "r"(31));

  //init_timer();
  //init_eip();

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
