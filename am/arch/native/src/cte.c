#include <am.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

extern void asm_trap();
extern void ret_from_trap();

extern void get_cur_as(_Context *c);
extern void _switch(_Context *c);

void irq_handle(_Context *c) {
  getcontext(&c->uc);
  get_cur_as(c);

  _Event e;
  e.event = ((uint32_t)c->rax == -1 ? _EVENT_YIELD : _EVENT_SYSCALL);
  _Context *ret = user_handler(e, c);
  if (ret != NULL) {
    c = ret;
  }

  _switch(c);
  c->uc.uc_mcontext.gregs[REG_RIP] = (uintptr_t)ret_from_trap;
  c->uc.uc_mcontext.gregs[REG_RSP] = (uintptr_t)c;

  setcontext(&c->uc);
}

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  void *start = (void *)0x100000;
  *(uintptr_t *)start = (uintptr_t)asm_trap;

  user_handler = handler;
  return 0;
}

void get_example_uc(_Context *r);

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)stack.end - 1;

  get_example_uc(c);
  c->rip = (uintptr_t)entry;
  return c;
}

void _yield() {
  asm volatile("call *0x100000": : "a"(-1));
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
