#include <am.h>
#include <riscv.h>
#include <klib.h>

// NMI-specific handler
static _Context* (*custom_nmi_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

#define INTR_BIT (1ul << (sizeof(uintptr_t) * 8 - 1))

/*
 * Default NMI handler
 */
_Context* __am_nmi_default_handler(_Event *ev, _Context *c) {
  printf("NMI detected, mncause=%llx, mnepc=%llx\n", c->scause, c->sepc);
  ev->event = _EVENT_IRQ_IODEV;  // Treat NMI as external interrupt
  _halt(2);
  return c;
}

/*
 * Main NMI handler
 */
_Context* __am_nmi_handle(_Context *c) {
  __am_get_cur_as(c);

  _Event ev = {0};

  printf("NMI handler called: mncause=%llx, mnepc=%llx\n", c->scause, c->sepc);

  if (custom_nmi_handler != NULL) {
    custom_nmi_handler(ev, c);
  } else {
    __am_nmi_default_handler(&ev, c);
  }

  __am_switch(c);

#if __riscv_xlen == 64
  asm volatile("fence.i");
#endif

  return c;
}

extern void __am_asm_nmi_trap(void);

/*
 * NMI handler register function
 */
void nmi_handler_reg(_Context*(*handler)(_Event, _Context*)) {
  custom_nmi_handler = handler;
}

/*
 * Initialize NMI handling
 */
int _nmi_init(_Context *(*handler)(_Event ev, _Context *ctx)) {
  // NMI shares mtvec with M-mode interrupts
  // Set M-mode trap vector to our NMI trap handler
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_nmi_trap));

  // Initialize mnscratch (0x740) to 0
  asm volatile("csrw 0x740, zero");

  // Register custom handler if provided
  if (handler != NULL) {
    custom_nmi_handler = handler;
  }

  printf("NMI initialized, mtvec set to %p\n", __am_asm_nmi_trap);

  return 0;
}
