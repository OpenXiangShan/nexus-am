#include <am.h>
#include <mips32.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void get_cur_as(_Context *c);
void _switch(_Context *c);
void tlb_refill(void);

_Context* irq_handle(_Context *c) {
  get_cur_as(c);

  _Context *next = c;
  if (user_handler) {
    _Event ev = {0};
    uint32_t ex_code = (c->cause >> 2) & 0x1f;
    uint32_t syscall_instr;
    switch (ex_code) {
      case 0: ev.event = _EVENT_IRQ_TIMER; break;
      case 2:
      case 3: tlb_refill(); return next;
      case 8: 
        syscall_instr = *(uint32_t *)(c->epc);
        ev.event = ((syscall_instr >> 6) == 1) ? _EVENT_YIELD : _EVENT_SYSCALL;
        c->epc += 4;
        break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, c);
    if (next == NULL) {
      next = c;
    }
  }

  _switch(next);

  return next;
}

extern void asm_trap(void);

#define EX_ENTRY 0x80000180

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  const uint32_t j_opcode = 0x08000000;
  uint32_t instr = j_opcode | (((uint32_t)asm_trap >> 2) & 0x3ffffff);
  *(uint32_t *)EX_ENTRY = instr;
  *(uint32_t *)0x80000000 = instr;  // TLB refill exception

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)stack.end - 1;

  c->epc = (uintptr_t)entry;
  return c;
}

void _yield() {
  asm volatile("syscall 1");
}

int _istatus(int enable) {
  return 0;
}
