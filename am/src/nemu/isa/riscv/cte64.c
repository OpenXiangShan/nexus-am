#include <riscv.h>
#include <nemu.h>

extern void __am_timervec(void);

static void init_machine_exception() {
  // set M-mode exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_timervec));
}


extern void init_timer();
extern void enable_timer();

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
  asm volatile("csrw pmpaddr0, %0" : : "r"(-1));
  asm volatile("csrw pmpcfg0, %0" : : "r"(31));

  init_machine_exception();
  init_timer();
  enable_timer();
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
