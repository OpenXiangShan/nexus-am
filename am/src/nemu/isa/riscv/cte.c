#include <am.h>
#include <nemu.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  if (user_handler) {
    _Event ev = {0};
    switch (c->cause) {
      case 0x80000005: ev.event = _EVENT_IRQ_TIMER; break;
      case 9:
        ev.event = (c->GPR1 == -1) ? _EVENT_YIELD : _EVENT_SYSCALL;
        c->epc += 4;
        break;
      default: ev.event = _EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  __am_switch(c);

  return c;
}

extern void __am_asm_trap(void);

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  asm volatile("csrw stvec, %0" : : "r"(__am_asm_trap));

  asm volatile("csrw sscratch, zero");

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area kstack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)kstack.end - 1;

  c->pdir = NULL;
  c->epc = (uintptr_t)entry;
  c->GPR2 = (uintptr_t)arg;
  c->status = 0x000c0100;
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
}
