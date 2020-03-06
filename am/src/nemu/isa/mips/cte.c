#include <nemu.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);
void __am_tlb_refill(void);

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  if (user_handler) {
    _Event ev = {0};
    uint32_t ex_code = (c->cause >> 2) & 0x1f;
    uint32_t syscall_instr;
    switch (ex_code) {
      case 0: ev.event = _EVENT_IRQ_TIMER; break;
      case 2:
      case 3: __am_tlb_refill(); return c;
      case 8:
        syscall_instr = *(uint32_t *)(c->epc);
        ev.event = ((syscall_instr >> 6) == 1) ? _EVENT_YIELD : _EVENT_SYSCALL;
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

#define EX_ENTRY 0x80000180

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  const uint32_t j_opcode = 0x08000000;
  uint32_t instr = j_opcode | (((uint32_t)__am_asm_trap >> 2) & 0x3ffffff);
  *(uint32_t *)EX_ENTRY = instr;
  *(uint32_t *)(EX_ENTRY + 4) = 0;  // delay slot
  *(uint32_t *)0x80000000 = instr;  // TLB refill exception
  *(uint32_t *)(0x80000000 + 4) = 0;  // delay slot

  asm volatile("move $k0, $zero");

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area kstack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)kstack.end - 1;
  c->pdir = NULL;
  c->epc = (uintptr_t)entry;
  c->GPR2 = (uintptr_t)arg; // a0
  c->gpr[29] = 0; // sp slot, used as usp
  return c;
}

void _yield() {
  asm volatile("syscall 1");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
