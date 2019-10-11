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
    switch (c->mcause) {
      case (0x7 | INTR_BIT):
        inc_mtimecmp(ind(CLINT_MTIME));
        ev.event = _EVENT_IRQ_TIMER;
        break;
      case 11:
        ev.event = (c->GPR1 == -1) ? _EVENT_YIELD : _EVENT_SYSCALL;
        c->mepc += 4;
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
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  inc_mtimecmp(0);
  asm volatile("csrs mie, %0" : : "r"(1 << 7));

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)stack.end - 1;

  c->mepc = (uintptr_t)entry;
  c->mstatus = 0x000c0180;
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
