#include <am.h>
#include <x86.h>
#include <am-x86.h>
#include <stdarg.h>

extern "C" { int printk(const char *, ...); }

static _RegSet* (*H)(_Event, _RegSet*) = nullptr;
static uint32_t args[4];

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

void irq_handle(TrapFrame *tf) {
  _RegSet regs = {
    .eax = tf->eax, .ebx = tf->ebx, .ecx = tf->ecx, .edx = tf->edx,
    .esi = tf->esi, .edi = tf->edi, .ebp = tf->ebp, .esp3 = 0,
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
    regs.esp3 = tf->esp;
    regs.ss0 = KSEL(SEG_KDATA);
    regs.esp0 = (uint32_t)tf + 68;
  } else { // interrupt at kernel code
    regs.ss0 = KSEL(SEG_KDATA);
    regs.esp0 = (uint32_t)tf + 60; // the %esp before interrupt
  }

  args[0] = regs.eax;
  args[1] = regs.edx;
  args[2] = regs.ecx;
  args[3] = regs.ebx;

  ev.event = _EVENT_NULL;
  if (tf->irq == 32) ev.event = _EVENT_IRQ_TIMER;
  else if (tf->irq == 33) ev.event = _EVENT_IRQ_IODEV;
  else if (tf->irq == 0x80) {
    if ((int32_t)tf->eax == -1) {
      ev.event = _EVENT_TRAP;
    } else {
      ev.event = _EVENT_SYSCALL;
    }
  } else if (tf->irq == 14) {
    uint32_t err = tf->err, cause = 0;
    ev.event = _EVENT_PAGEFAULT;
    if (err & 0x1) {
      cause |= _PROT_NONE;
    }
    if (err & 0x2) {
      cause |= _PROT_WRITE;
    } else {
      cause |= _PROT_READ;
    }
    ev.cause = cause;
    ev.ref = get_cr2();
  } else if (tf->irq < 32) ev.event = _EVENT_ERROR;

  _RegSet *ret = &regs;
  if (H) {
    _RegSet *next = H(ev, &regs);
    if (next != nullptr) {
      ret = next;
      ret->eax = args[0];
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
        "m"(ret->esp3),
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


void _asye_init(_RegSet*(*h)(_Event, _RegSet*)) {
  static GateDesc idt[NR_IRQ];
  smp_init();
  lapic_init();
  ioapic_enable(IRQ_KBD, 0);

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
  _RegSet *regs = (_RegSet*)stack.start;
  regs->esp0 = reinterpret_cast<uint32_t>(stack.end);
  regs->cs = KSEL(SEG_KCODE);
  regs->ds = regs->es = regs->ss = KSEL(SEG_KDATA);
  regs->eip = (uint32_t)entry;
  regs->eflags = FL_IF;
  regs->esp0 -= 4;
  *((void**)(regs->esp0)) = arg; // argument
  regs->esp0 -= 4;
  *((void**)(regs->esp0)) = NULL; // return address
  return regs;
}

void _trap() {
  asm volatile("int $0x80" : : "a"(-1));
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

