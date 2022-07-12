#include <amtest.h>
#include <xs.h>
#include <nemu.h>

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV64_XS) || defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
#define CLINT_SOFT_ADDRESS (RTC_ADDR - 0xbff8)
#else
#define CLINT_SOFT_ADDRESS 0xa2000000
#endif

_Context *soft_trap(_Event ev, _Context *ctx) {
  switch(ev.event) {
    case _EVENT_IRQ_SOFT:
      printf("s"); break;
    case _EVENT_IRQ_TIMER:
      printf("t"); break;
    case _EVENT_IRQ_IODEV:
      printf("d"); read_key(); break;
    case _EVENT_YIELD:
      printf("y"); break;
  }
  return ctx;
}

static void soft_intr_init() {
  // enable supervisor software interrupt (sie.ssie and sstatus.sie)
  asm volatile("csrs sstatus, %0" : : "r"(1 << 1));
  asm volatile("csrs sie, %0" : : "r"(1 << 1));
}

void soft_intr() {
    soft_intr_init();
    *(uint32_t *)(CLINT_SOFT_ADDRESS) = 1;
    asm volatile("csrw sip, 2");
}