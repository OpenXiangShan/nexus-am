#include "x86_64-qemu.h"
#include <stdarg.h>

#define IRQ    T_IRQ0 + 
#define MSG(m) ev.msg = m;

#define dump(name) \
  printf("%s = %08x (%d)\n", #name, ctx.name, ctx.name);

#define offset(name) \
  printf("%s @ %d\n", #name, (char *)&tf.name - (char*)tf);

void irq_x86(struct trap_frame *tf) {
  _Context ctx;
  printf("[%d/%d] ", tf->irq, _cpu());

#if __x86_64
  ctx        = tf->saved_context;
  ctx.rip    = tf->rip;
  ctx.cs     = tf->cs;
  ctx.rflags = tf->rflags;
  ctx.rsp    = tf->rsp;
  ctx.ss     = tf->ss;

  /*
  dump(rax); dump(rbx); dump(rcx); dump(rdx); dump(rbp); dump(rsp); dump(rsi); dump(rdi);
  dump(r8); dump(r9); dump(r10); dump(r11); dump(r12); dump(r13); dump(r14); dump(r15);
  dump(cs); dump(ss); dump(rip); dump(rflags);
  */
#else
  ctx     = tf->saved_context;
  ctx.eip = tf->eip;
  ctx.cs  = tf->cs;
  ctx.eflags = tf->eflags;
  if (tf->cs & DPL_USER) {
  } else {
    ctx.esp = (uint32_t)(tf + 1) - 8; // no ss/esp saved
    ctx.ss  = 0;
  }

/*
  dump(eax); dump(ebx); dump(ecx); dump(edx); 
  dump(ebp); dump(esp); dump(esi); dump(edi);
  dump(cs); dump(ds); dump(eip); dump(eflags); 
*/
#endif

  if (IRQ 0 <= tf->irq && tf->irq < IRQ 32) {
    __am_lapic_eoi();
  }

  __am_iret(&ctx);
}

void am_on_irq(struct trap_frame *tf) {
  stack_switch(stack_top(&CPU->irq_stack), irq_x86, (uintptr_t)tf);
}

static _Context* (*user_handler)(_Event, _Context*) = NULL;
#if __x86_64__
static GateDesc64 idt[NR_IRQ];
#define GATE GATE64
#else
static GateDesc32 idt[NR_IRQ];
#define GATE GATE32
#endif


#define IRQHANDLE_DECL(id, dpl, err)  void __am_irq##id();
IRQS(IRQHANDLE_DECL)
void __am_irqall();

int _cte_init(_Context *(*handler)(_Event, _Context *)) {
  if (_cpu() != 0) panic("init CTE in non-bootstrap CPU");

  for (int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG, KSEL(SEG_KCODE), __am_irqall, DPL_KERN);
  }
#define IDT_ENTRY(id, dpl, err) \
  idt[id] = GATE(STS_TG, KSEL(SEG_KCODE), __am_irq##id, DPL_##dpl);
  IRQS(IDT_ENTRY)

  user_handler = handler;
  return 0;
}

void _yield() {
  if (!user_handler) panic("no interrupt handler");
  asm volatile ("int $0x80" : : "a"(-1));
}

int _intr_read() {
  if (!user_handler) panic("no interrupt handler");
  return (get_efl() & FL_IF) != 0;
}

void _intr_write(int enable) {
  if (!user_handler) panic("no interrupt handler");
  if (enable) {
    sti();
  } else {
    cli();
  }
}

// static void panic_on_return() { panic("kernel context returns"); }

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  panic("!");
  return NULL;
}

/*
void __am_irq_handle(TrapFrame *tf) {
  if (tf->cs & DPL_USER) { // interrupt at user code
  } else { // interrupt at kernel code
  }

  // sending end-of-interrupt
  if (IRQ 0 <= tf->irq && tf->irq < IRQ 32) {
    __am_lapic_eoi();
  }

  // creating an event
  _Event ev = {
    .event = _EVENT_NULL,
    .cause = 0, .ref = 0,
    .msg = "(no message)",
  };
  
  switch (tf->irq) {
    case IRQ 0: MSG("timer interrupt (lapic)")
      ev.event = _EVENT_IRQ_TIMER; break;
    case IRQ 1: MSG("I/O device IRQ1 (keyboard)")
      ev.event = _EVENT_IRQ_IODEV; break;
    case EX_SYSCALL: MSG("int $0x80 trap: _yield() or system call")
      if ((int32_t)tf->eax == -1) {
        ev.event = _EVENT_YIELD;
      } else {
        ev.event = _EVENT_SYSCALL;
      }
      break;
    case EX_DIV: MSG("divide by zero")
      ev.event = _EVENT_ERROR; break;
    case EX_UD: MSG("UD #6 invalid opcode")
      ev.event = _EVENT_ERROR; break;
    case EX_NM: MSG("NM #7 coprocessor error")
      ev.event = _EVENT_ERROR; break;
    case EX_DF: MSG("DF #8 double fault")
      ev.event = _EVENT_ERROR; break;
    case EX_TS: MSG("TS #10 invalid TSS")
      ev.event = _EVENT_ERROR; break;
    case EX_NP: MSG("NP #11 segment/gate not present")
      ev.event = _EVENT_ERROR; break;
    case EX_SS: MSG("SS #12 stack fault")
      ev.event = _EVENT_ERROR; break;
    case EX_GP: MSG("GP #13, general protection fault")
      ev.event = _EVENT_ERROR; break;
    case EX_PF: MSG("PF #14, page fault, @cause: _PROT_XXX")
      ev.event = _EVENT_PAGEFAULT;
      if (tf->err & 0x1) ev.cause |= _PROT_NONE;
      if (tf->err & 0x2) ev.cause |= _PROT_WRITE;
      else               ev.cause |= _PROT_READ;
      ev.ref = get_cr2();
      break;
    default: MSG("unrecognized interrupt/exception")
      ev.event = _EVENT_ERROR;
      ev.cause = tf->err;
      break;
  }
}
*/

void __am_percpu_initirq() {
  __am_ioapic_enable(IRQ_KBD, 0);
  set_idt(idt, sizeof(idt));
}