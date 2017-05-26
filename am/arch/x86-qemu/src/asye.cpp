#include <am.h>
#include <x86.h>
#include <stdarg.h>

static _RegSet* (*H)(_Event, _RegSet*) = nullptr;

const int PORT_PIC_MS = 0x20;
const int PORT_PIC_SL = 0xA0;

extern "C" {
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

extern TSS tss[];

void lapic_eoi();

void irq_handle(TrapFrame *tf) {
  _RegSet regs = {
    .eax = tf->eax, .ebx = tf->ebx, .ecx = tf->ecx, .edx = tf->edx,
    .esi = tf->esi, .edi = tf->edi, .ebp = tf->ebp, .esp = 0,
    .eip = tf->eip, .eflags = tf->eflags,
    .cs = tf->cs, .ds = tf->ds, .es = tf->es, .ss = 0,
    .ss0 = 0, .esp0 = 0,
  };
  
  if (tf->irq >= 32 && tf->irq < 64) {
    lapic_eoi();
  }


  _Event ev;
  
  if (tf->cs & DPL_USER) { // interrupt at user code
    regs.ss = tf->ss;
    regs.esp = tf->esp;
    regs.ss0 = KSEL(SEG_KDATA);
    regs.esp0 = (u32)tf + 68;
  } else { // interrupt at kernel code
    regs.ss0 = KSEL(SEG_KDATA);
    regs.esp0 = (u32)tf + 60; // the %esp before interrupt
  }

  ev.event = _EVENT_NULL;
  if (tf->irq == 32) ev.event = _EVENT_IRQ_TIME;
  else if (tf->irq == 0x80) ev.event = _EVENT_SYSCALL;
  else if (tf->irq < 32) ev.event = _EVENT_ERROR;

  _RegSet *ret = &regs;
  if (H) {
    _RegSet *next = H(ev, &regs);
    if (next != nullptr) {
      ret = next;
    }
  }

  if (ret->cs & DPL_USER) {
    tss[_cpu()].ss0 = ret->ss0;
    tss[_cpu()].esp0 = ret->esp0;
    // return to user
    asm volatile(
      "nop;"
      "movl %0, %%esp;" // move stack
      "push %1;" // SS
      "push %2;" // ESP
      "push %3;" // EFL
      "push %4;" // CS
      "push %5;" // EIP
      "push %6;" // DS
      "push %7;" // ES
      "push %8;" // EAX  - popal
      "push %9;" // ECX
      "push %10;" // EDX
      "push %11;" // EBX
      "push %12;" // ESP
      "push %13;" // EBP
      "push %14;" // ESI
      "push %15;" // EDI -
      "popal;"
      "popl %%es;"
      "popl %%ds;"
      "iret;"
      "nop;"
    : : "m"(ret->esp0),
        "m"(ret->ss),
        "m"(ret->esp),
        "m"(ret->eflags),
        "m"(ret->cs),
        "m"(ret->eip),
        "m"(ret->ds),
        "m"(ret->es),
        "m"(ret->eax),
        "m"(ret->ecx),
        "m"(ret->edx),
        "m"(ret->ebx),
        "m"(ret->esp0),
        "m"(ret->ebp),
        "m"(ret->esi),
        "m"(ret->edi)
    );

  } else {
    // return to kernel
    asm volatile(
      "nop;"
      "movl %0, %%esp;" // move stack
      "push %1;" // EFL
      "push %2;" // CS
      "push %3;" // EIP
      "push %4;" // DS
      "push %5;" // ES
      "push %6;" // EAX  - popal
      "push %7;" // ECX
      "push %8;" // EDX
      "push %9;" // EBX
      "push %10;" // ESP
      "push %11;" // EBP
      "push %12;" // ESI
      "push %13;" // EDI -
      "popal;"
      "popl %%es;"
      "popl %%ds;"
      "iret;"
      "nop;"
    : : "m"(ret->esp0),
        "m"(ret->eflags),
        "m"(ret->cs),
        "m"(ret->eip),
        "m"(ret->ds),
        "m"(ret->es),
        "m"(ret->eax),
        "m"(ret->ecx),
        "m"(ret->edx),
        "m"(ret->ebx),
        "m"(ret->esp0),
        "m"(ret->ebp),
        "m"(ret->esi),
        "m"(ret->edi)
    );
  }
}

}

static GateDesc idt[NR_IRQ];

void _asye_init() {
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
}

void _idle() {
  hlt();
}

void _listen(_RegSet*(*h)(_Event, _RegSet*)) {
  H = h;
}

_RegSet *_make(_Area stack, void *entry) {
  _RegSet *regs = (_RegSet*)stack.start;
  regs->esp = reinterpret_cast<u32>(stack.end);
  regs->cs = KSEL(SEG_KCODE);
  regs->ds = regs->es = regs->ss = KSEL(SEG_KDATA);
  regs->eip = (u32)entry;
  regs->eflags = FL_IF;
  return regs;
}

void _trap() {
  asm volatile("int $0x80");
}

void _idisable() {
  cli();
}
void _ienable() {
  sti();
}
int _istatus() {
  return (get_efl() & FL_IF) != 0;
}
