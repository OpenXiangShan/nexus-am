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
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), irqall, DPL_KERN);
  }

  // --------------------- exceptions --------------------------
  idt[0]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec0,   DPL_KERN);
  idt[1]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec1,   DPL_KERN);
  idt[2]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec2,   DPL_KERN);
  idt[3]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec3,   DPL_KERN);
  idt[4]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec4,   DPL_KERN);
  idt[5]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec5,   DPL_KERN);
  idt[6]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec6,   DPL_KERN);
  idt[7]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec7,   DPL_KERN);
  idt[8]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec8,   DPL_KERN);
  idt[9]    = GATE(STS_TG32, KSEL(SEG_KCODE), vec9,   DPL_KERN);
  idt[10]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec10,  DPL_KERN);
  idt[11]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec11,  DPL_KERN);
  idt[12]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec12,  DPL_KERN);
  idt[13]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec13,  DPL_KERN);
  idt[14]   = GATE(STS_TG32, KSEL(SEG_KCODE), vec14,  DPL_KERN);
  // --------------------- interrupts --------------------------
  idt[32]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq0,   DPL_KERN);
  idt[33]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq1,   DPL_KERN);
  idt[46]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq14,  DPL_KERN);
  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_USER);
  set_idt(idt, sizeof(idt));
  H = h;
}

_RegSet *_make(_Area stack, void *entry, void *arg) {
  // TODO: pass arg
  /*
  _RegSet *regs = (_RegSet*)stack.start;
  regs->esp0 = reinterpret_cast<uint32_t>(stack.end);
  regs->cs = KSEL(SEG_KCODE);
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
