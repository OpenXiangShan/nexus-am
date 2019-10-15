// unmodified for cputest

#include <am.h>
#include <riscv64.h>
#include <klib.h>

#define CLINT_MMIO 0xa2000000
#define CLINT_MTIMECMP (CLINT_MMIO + 0x4000)
#define CLINT_MTIME    (CLINT_MMIO + 0xbff8)
#define TIME_INC 0x800
static inline void inc_mtimecmp(uint64_t this) {
  outd(CLINT_MTIMECMP, this + TIME_INC);
}

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

#define INTR_BIT (1ULL << 63)

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  _Context *next = c;
  if (user_handler) {
    _Event ev = {0};
    switch (c->cause) {
      case (0x7 | INTR_BIT):
        inc_mtimecmp(ind(CLINT_MTIME));
        ev.event = _EVENT_IRQ_TIMER;
        break;
      case 9:
        ev.event = (c->GPR1 == -1) ? _EVENT_YIELD : _EVENT_SYSCALL;
        c->epc += 4;
        break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, c);
    if (next == NULL) {
      next = c;
    }
  }

  __am_switch(next);

  return next;
}

extern void __am_asm_trap(void);

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  asm volatile("csrw stvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  // set machine timer interrupt
  inc_mtimecmp(0);
  asm volatile("csrs mie, %0" : : "r"(1 << 7));

  // set delegation
  asm volatile("csrw mideleg, %0" : : "r"(0xffff));
  asm volatile("csrw medeleg, %0" : : "r"(0xffff));

  // enter S-mode
  uintptr_t status = MSTATUS_MXR | MSTATUS_SUM | MSTATUS_SPP(MODE_S);
  extern char _here;
  asm volatile(
    "csrw sstatus, %0;"
    "csrw sepc, %1;"
    "sret;"
    "_here:"
    : : "r"(status), "r"(&_here));

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)stack.end - 1;

  c->epc = (uintptr_t)entry;
  uintptr_t mprotect = MSTATUS_MXR | MSTATUS_SUM;
  c->status = mprotect | MSTATUS_SPP(MODE_S) | MSTATUS_PIE(MODE_S);
  return c;
}

void _yield() {
  asm volatile("li a7, -1; ecall");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
