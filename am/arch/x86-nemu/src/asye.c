#include <am.h>
#include <x86.h>

static _RegSet* (*H)(_Event, _RegSet*) = NULL;
static uint32_t args[4];

void irq0();
void irq1();
void irq14();
void vec0();
void vec1();
void vec2();
void vec3();
void vec4();
void vec5();
void vec6();
void vec7();
void vec8();
void vec9();
void vec10();
void vec11();
void vec12();
void vec13();
void vec14();
void vecsys();
void irqall();

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

  // init IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(0, 0, irqall, 0);
  }

  // --------------------- exceptions --------------------------
  idt[0]    = GATE(0, 0, vec0,   0);
  idt[1]    = GATE(0, 0, vec1,   0);
  idt[2]    = GATE(0, 0, vec2,   0);
  idt[3]    = GATE(0, 0, vec3,   0);
  idt[4]    = GATE(0, 0, vec4,   0);
  idt[5]    = GATE(0, 0, vec5,   0);
  idt[6]    = GATE(0, 0, vec6,   0);
  idt[7]    = GATE(0, 0, vec7,   0);
  idt[8]    = GATE(0, 0, vec8,   0);
  idt[9]    = GATE(0, 0, vec9,   0);
  idt[10]   = GATE(0, 0, vec10,  0);
  idt[11]   = GATE(0, 0, vec11,  0);
  idt[12]   = GATE(0, 0, vec12,  0);
  idt[13]   = GATE(0, 0, vec13,  0);
  idt[14]   = GATE(0, 0, vec14,  0);
  // --------------------- interrupts --------------------------
  idt[32]   = GATE(0, 0, irq0,   0);
  idt[33]   = GATE(0, 0, irq1,   0);
  idt[46]   = GATE(0, 0, irq14,  0);
  // -------------------- system call --------------------------
  idt[0x80] = GATE(0, 0, vecsys, 0);
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
  asm volatile("int $0x80");
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
