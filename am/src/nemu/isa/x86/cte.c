#include <am.h>
#include <nemu.h>
#include <klib.h>

#define NR_IRQ         256     // IDT size
#define SEG_KCODE      1
#define SEG_KDATA      2

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_irq0();
void __am_vecsys();
void __am_vectrap();
void __am_vecnull();

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

uintptr_t __am_ksp = 0;

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  if (__am_ksp != 0) {
    // trap from user
    memcpy(&c->irq, (void *)__am_ksp, 5 * sizeof(uintptr_t));
    c->usp = __am_ksp + 5 * sizeof(uintptr_t);
    __am_ksp = 0;
  }

  if (user_handler) {
    _Event ev = {0};
    switch (c->irq) {
      case 32: ev.event = _EVENT_IRQ_TIMER; break;
      case 0x80: ev.event = _EVENT_SYSCALL; break;
      case 0x81: ev.event = _EVENT_YIELD; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  __am_switch(c);

  if (c->usp != 0) {
    // return to user, set ksp for the next use
    __am_ksp = (uintptr_t)(c + 1);
  }

  return c;
}

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  static GateDesc32 idt[NR_IRQ];

  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i]  = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecnull, DPL_KERN);
  }

  // ----------------------- interrupts ----------------------------
  idt[32]   = GATE32(STS_IG, KSEL(SEG_KCODE), __am_irq0,    DPL_KERN);
  // ---------------------- system call ----------------------------
  idt[0x80] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vecsys,  DPL_USER);
  idt[0x81] = GATE32(STS_TG, KSEL(SEG_KCODE), __am_vectrap, DPL_KERN);

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

void __am_kcontext_start();

_Context* _kcontext(_Area kstack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context *)kstack.end - 1;
  c->cr3 = NULL;
  c->cs = 0x8;
  c->eip = (uintptr_t)__am_kcontext_start;
  c->eflags = 0x2 | FL_IF;
  c->usp = 0;
  c->GPR1 = (uintptr_t)arg;
  c->GPR2 = (uintptr_t)entry;
  return c;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
