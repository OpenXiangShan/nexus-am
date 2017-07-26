#include <am.h>
#include <x86.h>

static _RegSet* (*H)(_Event, _RegSet*) = NULL;
static uint32_t args[4];

void irq0();
void vecsys();
void vectrap();
void vecnull();

void irq_handle(TrapFrame *tf) {
  _RegSet *regs = tf;
  _Event ev;

  args[0] = regs->eax;
  args[1] = regs->edx;
  args[2] = regs->ecx;
  args[3] = regs->ebx;

  ev.event = _EVENT_NULL;
  if (tf->irq == 32) ev.event = _EVENT_IRQ_TIME;
  else if (tf->irq == 33) ev.event = _EVENT_IRQ_IODEV;
  else if (tf->irq == 0x80) {
    ev.event = _EVENT_TRAP;
  }
  else if (tf->irq < 32) ev.event = _EVENT_ERROR;

  if (H) {
    H(ev, regs);
  }

}

static GateDesc idt[NR_IRQ];

void _asye_init(_RegSet*(*h)(_Event, _RegSet*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // --------------------- interrupts --------------------------
  idt[32]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq0,   DPL_KERN);
  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_USER);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);
  set_idt(idt, sizeof(idt));
  H = h;
}

_RegSet *_make(_Area stack, void *entry, void *arg) {
  // TODO: pass arg
  /*
  _RegSet *regs = (_RegSet*)stack.start;
  regs->esp0 = reinterpret_cast<uint32_t>(stack.end);
  regs->cs = 0;
  regs->ds = regs->es = regs->ss = KSEL(SEG_KDATA);
  regs->eip = (uint32_t)entry;
  regs->eflags = FL_IF;
  return regs;
  */
  return NULL;
}

void _trap() {
  asm volatile("int $0x81");
}

int _istatus(int enable) {
  int ret = (get_efl() & FL_IF) != 0;
  if (enable) {
    sti();
  } else {
    cli();
  }
  return ret;
}
