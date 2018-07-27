#include <am.h>
#include <klib.h>

static _RegSet* (*H)(_Event, _RegSet*) = NULL;

extern void asm_trap();
extern void ret_from_trap();

void irq_handle(_RegSet *r) {
  getcontext(&r->uc);

  _Event e;
  e.event = ((uint32_t)r->rax == -1 ? _EVENT_YIELD : _EVENT_SYSCALL);
  _RegSet *ret = H(e, r);
  if (ret != NULL) {
    r = ret;
  }

  r->uc.uc_mcontext.gregs[REG_RIP] = (uintptr_t)ret_from_trap;
  r->uc.uc_mcontext.gregs[REG_RSP] = (uintptr_t)r;

  setcontext(&r->uc);
}

int _asye_init(_RegSet*(*handler)(_Event, _RegSet*)) {
  void *start = (void *)0x100000;
  *(uintptr_t *)start = (uintptr_t)asm_trap;

  H = handler;
  return 0;
}

_RegSet *_make(_Area stack, void (*entry)(void *), void *arg) {
  return NULL;
}

void _yield() {
  asm volatile("call *0x100000": : "a"(-1));
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
