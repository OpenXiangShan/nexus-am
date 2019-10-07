#include <am.h>
#include <riscv64.h>
#include <klib.h>

#define CLINT_MMIO 0x40700000
#define CLINT_MTIME    (CLINT_MMIO + 0)
#define CLINT_MTIMECMP (CLINT_MMIO + 8)
#define TIME_INC 10
static inline void inc_mtimecmp(uint64_t this) {
  outd(CLINT_MTIMECMP, this + TIME_INC);
}

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

int __am_illegal_instr(_Context *c);

#define INTR_BIT (1ULL << 63)

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  _Context *next = c;
  if (user_handler) {
    _Event ev = {0};
    switch (c->mcause) {
      case 2:
        if (__am_illegal_instr(c)) c->mepc += 4;
        break;
      case (0x7 | INTR_BIT):
        inc_mtimecmp(ind(CLINT_MTIME));
        ev.event = _EVENT_IRQ_TIMER;
        break;
      case (0xb | INTR_BIT):
        ev.event = _EVENT_IRQ_IODEV;
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

//  __am__switch(next);

  return next;
}

extern void __am_asm_trap(void);

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  inc_mtimecmp(0);
  asm volatile("csrw mie, %0" : : "r"((1 << 7) | (1 << 11)));

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
  uintptr_t mstatus;
  asm volatile("csrr %0, mstatus" : "=r"(mstatus));
  return (mstatus & 0x8) != 0;
}

void _intr_write(int enable) {
  uintptr_t mstatus;
  asm volatile("csrr %0, mstatus" : "=r"(mstatus));
  if (enable) {
    mstatus |= 0x8;
  } else {
    mstatus &= ~0x8;
  }
  asm volatile("csrw mstatus, %0" : : "r"(mstatus));
}
