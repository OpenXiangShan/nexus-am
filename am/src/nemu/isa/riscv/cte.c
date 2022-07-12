#include <am.h>
#include <riscv.h>
#include <klib.h>

// static _Context* (*user_handler)(_Event, _Context*) = NULL;
static _Context* (*custom_soft_handler)(_Event, _Context*) = NULL;
static _Context* (*custom_timer_handler)(_Event, _Context*) = NULL;
static _Context* (*custom_external_handler)(_Event, _Context*) = NULL;
static _Context* (*custom_secall_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

#define INTR_BIT (1ul << (sizeof(uintptr_t) * 8 - 1))

_Context* (*interrupt_handler[INTERRUPT_CAUSE_SIZE])(_Event *ev, _Context *c);
_Context* (*exception_handler[EXCEPTION_CAUSE_SIZE])(_Event *ev, _Context *c);
/*
 * default handler for all possible irqs
 * just panic
 */
_Context* __am_irq_default_handler(_Event *ev, _Context *c) {
  printf("unregisted irq detected, scause=%d, sepc=%llx\n", c->scause, c->sepc);
  ev->event = _EVENT_ERROR;
  _halt(2);
  // should never reach here
  return c;
}

/*
 * default handler for Supervisor Software Interrupt
 * set event to IRQ_SOFT
 * may call custom soft handler if registered
 */
_Context* __am_irq_SSIP_handler(_Event *ev, _Context *c) {
#if __riscv_xlen == 64
  asm volatile ("csrwi sip, 0");
#endif
  // printf("inside irq SSIP handler\n");
  ev->event = _EVENT_IRQ_SOFT;
  if (custom_soft_handler != NULL) {
    // printf("dive into custom soft handler");
    custom_soft_handler(*ev, c);
  }
  // machine mode will clear stip
  asm volatile("csrs mie, 0");
  // printf("SSIP handler finished\n");
  return c;
}

/*
 * default handler for Supervisor Timer Interrupt
 * set event to IRQ_TIMER
 * may call custom timer handler if registered
 */
_Context* __am_irq_STIP_handler(_Event *ev, _Context *c) {
#if __riscv_xlen == 64
  asm volatile ("csrwi sip, 0");
#endif
  // printf("inside irq STIP handler\n");
  ev->event = _EVENT_IRQ_TIMER;
  if (custom_timer_handler != NULL) {
    // printf("dive into custom timer handler");
    custom_timer_handler(*ev, c);
  }
  // machine mode will clear stip
  asm volatile("csrs mie, 0");
  // printf("STIP handler finished\n");
  return c;
}

/*
 * default handler for Supervisor External Interrupt
 * set event to IEQ_IODEV
 * may call custom external handler if registered
 */
_Context* __am_irq_SEIP_handler(_Event *ev, _Context *c) {
  // WARNING: this has no effect since in S mode only SSIP can be cleared.
  // It's not deleted because we want to test sip write mask.
  asm volatile ("csrwi sip, 0");
  ev->event = _EVENT_IRQ_IODEV;
  printf("inside irq SEIP handler\n");
  if (custom_external_handler != NULL)
    custom_external_handler(*ev, c);
  return c;
}

/*
 * default handler for Supervisor Ecall Exception
 * set event to YIELD or SYSCALL according to a7
 * may call custom secall handler if registered
 */
_Context* __am_irq_SECALL_handler(_Event *ev, _Context *c) {
  ev->event = (c->GPR1 == -1) ? _EVENT_YIELD : _EVENT_SYSCALL;
  c->sepc += 4;
  //if (ev->event == _EVENT_YIELD)
  //  printf("SECALL: is YIELD\n");
  // printf("Inside secall handler\n");
  if (custom_secall_handler != NULL) {
    custom_secall_handler(*ev, c);
  }
  return c;
}


_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  _Event ev = {0};
  uintptr_t scause_code = c->scause & SCAUSE_MASK;
  // printf("am irq triggered %llx\n", c->scause);
  if (c->scause & INTR_BIT) {
    assert(scause_code < INTERRUPT_CAUSE_SIZE);
    // printf("is an interrupt\n");
    interrupt_handler[scause_code](&ev, c);
  } else {
    assert(scause_code < EXCEPTION_CAUSE_SIZE);
    // printf("is an exception\n");
    exception_handler[scause_code](&ev, c);
  }

  __am_switch(c);

#if __riscv_xlen == 64
  asm volatile("fence.i");
#endif

  return c;
}

extern void __am_asm_trap(void);

/*
 * Supervisor soft interrupt custom handler register function
 * handler: the function to be registered
 */
void ssip_handler_reg(_Context*(*handler)(_Event, _Context*)) {
  custom_soft_handler = handler;
}

/*
 * Supervisor timer interrupt custom handler register function
 * handler: the function to be registered
 */
void stip_handler_reg(_Context*(*handler)(_Event, _Context*)) {
  custom_timer_handler = handler;
}

/*
 * Supervisor external interrupt custom handler register function
 * handler: the function to be registered
 */
void seip_handler_reg(_Context*(*handler)(_Event, _Context*)) {
  custom_external_handler = handler;
}

/*
 * Supervisor ecall exception custom handler register function
 * handler: the function to be registered
 */
void secall_handler_reg(_Context*(*handler)(_Event, _Context*)) {
  custom_secall_handler = handler;
}

/*
 * Generic interrupt/exception CUSTOM handler register function
 * code: scause code
 * handler: the function to be registered
 */
void custom_handler_reg(uintptr_t code, _Context*(*handler)(_Event, _Context*)) {
  switch (code) {
#if __riscv_xlen == 64
    case INTR_BIT | SCAUSE_SSIP:
      ssip_handler_reg(handler);
      break;
#endif
    case INTR_BIT | SCAUSE_STIP:
      stip_handler_reg(handler);
      break;
    case INTR_BIT | SCAUSE_SEIP:
      seip_handler_reg(handler);
      break;
    case SCAUSE_SECALL:
      secall_handler_reg(handler);
      break;
    default:
      printf("Unrecognized code, custom handler reg ignored\n");
      _halt(2);
  }
}

/*
 * Generic interrupt/exception handler register function
 * code: scause code
 * handler: the function to be registered
 */
void irq_handler_reg(uintptr_t code, _Context*(*handler)(_Event*, _Context*)) {
  uintptr_t offset = (code << 1) >> 1;
  if (INTR_BIT & code) {
    assert(offset < INTERRUPT_CAUSE_SIZE);
    interrupt_handler[offset] = handler;
  } else {
    assert(offset < EXCEPTION_CAUSE_SIZE);
    exception_handler[offset] = handler;
  }
}

int _cte_init(_Context *(*handler)(_Event ev, _Context *ctx)) {
  // initialize exception entry
  asm volatile("csrw stvec, %0" : : "r"(__am_asm_trap));

  asm volatile("csrw sscratch, zero");

  // cte init handler has no effect for now

#if __riscv_xlen == 64
  // printf("CTE64 inited\n");
  extern void __am_init_cte64();
  __am_init_cte64();
#endif
  for (int i = 0; i < INTERRUPT_CAUSE_SIZE; i++) {
    irq_handler_reg(INTR_BIT | i, __am_irq_default_handler);
    // interrupt_handler[i] = __am_irq_default_handler;
  }
  for (int i = 0; i < EXCEPTION_CAUSE_SIZE; i++) {
    exception_handler[i] = __am_irq_default_handler;
  }

#if __riscv_xlen == 64
  interrupt_handler[SCAUSE_SSIP] = __am_irq_SSIP_handler;
#endif
  interrupt_handler[SCAUSE_STIP] = __am_irq_STIP_handler;
  interrupt_handler[SCAUSE_SEIP] = __am_irq_SEIP_handler;
  exception_handler[SCAUSE_SECALL] = __am_irq_SECALL_handler;

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
  //printf("before ecall\n");
  asm volatile("li a7, -1; ecall");
  //printf("after ecall\n");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
	if (enable) asm volatile("csrs sstatus, 0x2");
	else asm volatile("csrc sstatus, 0x2");
}
