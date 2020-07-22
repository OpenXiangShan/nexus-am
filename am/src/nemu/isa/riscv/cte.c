#include <am.h>
#include <riscv.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

#define INTR_BIT (1ul << (sizeof(uintptr_t) * 8 - 1))

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  if (user_handler) {
    _Event ev = {0};
    switch (c->scause) {
#if __riscv_xlen == 64
      // SSIP, which is set at mtime.S
      case 0x1 | INTR_BIT: asm volatile ("csrwi sip, 0");
#else
      case 0x5 | INTR_BIT:
#endif
        ev.event = _EVENT_IRQ_TIMER; break;
      case 9:
        ev.event = (c->GPR1 == -1) ? _EVENT_YIELD : _EVENT_SYSCALL;
        c->sepc += 4;
        break;
      default: ev.event = _EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  __am_switch(c);

#if __riscv_xlen == 64
  asm volatile("fence.i");
#endif

  return c;
}

extern void __am_asm_trap(void);

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  asm volatile("csrw stvec, %0" : : "r"(__am_asm_trap));

  asm volatile("csrw sscratch, zero");

  // register event handler
  user_handler = handler;

#if __riscv_xlen == 64
  extern void __am_init_cte64();
  __am_init_cte64();
#endif

  return 0;
}

_Context *_kcontext(_Area kstack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)kstack.end - 1;

  c->pdir = NULL;
  c->sepc = (uintptr_t)entry;
  c->GPR2 = (uintptr_t)arg;
  c->sstatus = MSTATUS_SPP(MODE_S) | MSTATUS_PIE(MODE_S);
  c->gpr[2] = 0; // sp slot, used as usp
  return c;
}

void _yield() {
  asm volatile("li a7, -1; ecall");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
	if (enable) asm volatile("csrs sstatus, 0x2");
	else asm volatile("csrc sstatus, 0x2");
}
