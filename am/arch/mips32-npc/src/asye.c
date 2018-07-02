#include <am.h>
#include <npc.h>
#include <arch.h>
#include <klib.h>

static _RegSet* (*H) (_Event, _RegSet*) = NULL;

static void update_timer(int step) {
  int compare = 0;
  MFC0(compare, CP0_COMPARE, 0);
  compare += step;
  MTC0(CP0_COMPARE, compare, 0);
}

static void init_timer(int step) {
  int compare = 0;
  MFC0(compare, CP0_COUNT, 0);
  compare += step;
  MTC0(CP0_COMPARE, compare, 0);
}

int _asye_init(_RegSet* (*l)(_Event ev, _RegSet *regs)){
  H = l;
  init_timer(30000);
  return 0;
}

_RegSet *_make(_Area kstack, void (*entry)(void *), void *args){
  _RegSet *regs = (_RegSet *)kstack.start;
  regs->sp = (uint32_t) kstack.end;
  regs->epc = (uint32_t) entry;

  static const char *envp[] = { "FUCKYOU=true", NULL };

  uintptr_t *arg = args;
  regs->a0 = 0;
  regs->a1 = (uintptr_t)arg;
  regs->a2 = (uintptr_t)envp;
  for(; *arg; arg ++, regs->a0++);
  return regs;
}

void _yield(){
  asm volatile("addiu $a0, $0, -1; syscall; nop");
}

int _intr_read() {
  int status = 0;
  MFC0(status, CP0_STATUS, 0);
  return status & 0x1;
}

void _intr_write(int enable) {
  int status = 0;
  MFC0(status, CP0_STATUS, 0);
  status = status ^ (!!enable + 0);
  MTC0(CP0_STATUS, status, 0); 
}

void irq_handle(struct TrapFrame *tf){
  _RegSet regs = {
    .at=tf->at, 
    .v0 = tf->v0, .v1 = tf->v1,
    .a0 = tf->a0, .a1 = tf->a1, .a2 = tf->a2, .a3 = tf->a3,
    .t0 = tf->t0, .t1 = tf->t1, .t2 = tf->t2, .t3 = tf->t3,
    .t4 = tf->t4, .t5 = tf->t5, .t6 = tf->t6, .t7 = tf->t7,  
    .t8 = tf->t8, .t9 = tf->t9,  
    .s0 = tf->s0, .s1 = tf->s1, .s2 = tf->s2, .s3 = tf->s3, 
    .s4 = tf->s4, .s5 = tf->s5, .s6 = tf->s6, .s7 = tf->s7, 
    .k0 = tf->k0, .k1 = tf->k1,
    .gp = tf->gp, .sp = tf->sp, .fp = tf->fp, .ra = tf->ra,
    .epc = tf->epc, .cause = tf->cause, .status = tf->status, .badvaddr = tf->badvaddr,
  };

  cp0_cause_t *cause = (void*)&(regs.cause);
  uint32_t exccode = cause->ExcCode;
  uint32_t ipcode = cause->IP;

  _Event ev;
  ev.event = _EVENT_NULL;
  //TODO: exception handling
  // Delayslot should be considered when handle exceptions !!!
  switch(exccode){
    case EXC_INTR: {
      if(ipcode & IP_TIMER_MASK) {
		  update_timer(30000);
          ev.event = _EVENT_IRQ_TIMER;
	  } else {
		  printk("invalid ipcode = %x\n", ipcode);
		  _halt(-1);
      }
      break;
    }
    case EXC_SYSCALL:
      regs.epc += 4;
	  if(tf->a0 == -1)
		ev.event = _EVENT_YIELD;
	  else
		ev.event = _EVENT_SYSCALL;
      break;
    case EXC_TRAP:
      ev.event = _EVENT_SYSCALL;
      break;
    case EXC_AdEL:
    case EXC_AdES:
    case EXC_BP:
    case EXC_RI:
    case EXC_OV:
    default:
	  printk("unhandled exccode = %x, epc:%08x\n", exccode, regs.epc);
	  _halt(-1);
  }

  _RegSet *ret = &regs;
  if(H) {
	  _RegSet *next = H(ev, &regs);
	  if(next != NULL) ret = next;
  }

  asm volatile(
    "nop;"
    "lw $at, %0;"
    "lw $v0, %1;"
    "lw $a0, %3;"
    "lw $a1, %4;"
    "lw $a2, %5;"
    "lw $a3, %6;"
    "lw $s0, %7;"
    "lw $s1, %8;"
    "lw $s2, %9;"
    "lw $s3, %10;"
    "lw $s4, %11;"
    "lw $s5, %12;"
    "lw $s6, %13;"
    "lw $s7, %14;"
    "lw $gp, %15;"
    "lw $fp, %16;"
    "lw $ra, %17;"
    "lw $sp, %18;"
    "lw $k0, %19;"
    "nop;"
    "nop;"
    "mtc0 $k0, $14;"
    "nop;"
    "nop;"
    "lw $k0, %20;"
    "mtc0 $k0, $13;"
    "nop;"
    "nop;"
    "lw $k0, %21;"
    "mtc0 $k0, $12;"
    "nop;"
    "nop;"
    "lw $k0, %22;"
    "mtc0 $k0, $8;"
    "nop;"
    "nop;"
    "lw $v1, %2;"
    "eret;"
    : : 
    "m"(ret->at),
    "m"(ret->v0),
    "m"(ret->v1),
    "m"(ret->a0),
    "m"(ret->a1),
    "m"(ret->a2),
    "m"(ret->a3),
    "m"(ret->s0),
    "m"(ret->s1),
    "m"(ret->s2),
    "m"(ret->s3),
    "m"(ret->s4),
    "m"(ret->s5),
    "m"(ret->s6),
    "m"(ret->s7),
    "m"(ret->gp),
    "m"(ret->fp),
    "m"(ret->ra),
    "m"(ret->sp),
    "m"(ret->epc),
    "m"(ret->cause),
    "m"(ret->status),
    "m"(ret->badvaddr)
    :"at",
     "v0",
     "a0", "a1","a2","a3",
     "s0","s1","s2","s3","s4","s5","s6","s7",
     "fp","ra","sp"
    );
}
