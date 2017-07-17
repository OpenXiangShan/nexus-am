#include <am.h>
#include <npc.h>
#include <arch.h>
#include <klib.h>

static _RegSet* (*H) (_Event, _RegSet*) = NULL;
static uint32_t args[4];

void _asye_init(_RegSet* (*l)(_Event ev, _RegSet *regs)){
  H = l;
  // volatile uint32_t init = 0x10000;
  // SetCompare(init);
}

_RegSet *_make(_Area kstack, void *entry, void *arg){
  _RegSet *r = (void *)0;
  int status = 0;
  int new_status = 0;
  MFC0(status, cp0_status, 0);
  new_status = status | 0x1;
  MTC0(cp0_status, new_status, 0);  
  return r;
}

void _trap(){
  asm  volatile("syscall");
}

/*void _idisable(){
  int status = 0;
  MFC0(status, cp0_status, 0);
  status = status | 0x2;
  MTC0(cp0_status, status, 0);  
}*/

/*void _ienable(){
  int status = 0;
  MFC0(status, cp0_status, 0);
  status = status & 0xfffffffd;
  MTC0(cp0_status, status, 0);  
}*/

int _istatus(int enable){
  int status = 0;
  int new_status = 0;
  if(enable) {
    MFC0(status, cp0_status, 0);
    new_status = status | 0x1;
    MTC0(cp0_status, new_status, 0);  
  }
  else {
    MFC0(status, cp0_status, 0);
    new_status = status & 0xfffffffe;
    MTC0(cp0_status, new_status, 0); 
  }
  return status & 0x1;
}

void irq_handle(struct TrapFrame *tf){
//TODO:handle interrupt
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
  uint32_t ipcode, exccode;
  exccode = (regs.cause & 0xff) >> 2;
  ipcode = ((regs.cause & regs.status) & 0xff00) >> 8;
  args[0] = regs.a0;
  args[1] = regs.a1;
  args[2] = regs.a2;
  args[3] = regs.a3;

  _Event ev;
  ev.event = _EVENT_NULL;
  //TODO: exception handling
  // Delayslot should be considered when handle exceptions !!!
  switch(exccode){
    case 0:{ // interruption
      switch(ipcode){
        case 0x80:{ // time interrupt
          uint32_t count = GetCount(0);
          SetCompare(count + INTERVAL);
          ev.event = _EVENT_IRQ_TIME;
          break;
        }
        default:printk("ipcode = %x\n", ipcode);_halt(-1);
      }
      break;
    }
    case 4:{ //AdEL
      printk("AdEL\n");
      printk("$epc = 0x%x\n", regs.epc);
      printk("$BadVaddr = 0x%x\n", regs.badvaddr);
      _halt(-1);
      break;
    }
    case 5:{ //AdES
      printk("AdES\n");
      printk("$epc = 0x%x\n", regs.epc);
      printk("$BadVaddr = 0x%x\n", regs.badvaddr);
      _halt(-1);
      break;
    }
    case 8:{ // syscall
      tf->epc += 4;
      ev.event = _EVENT_TRAP;
	    printk("Syscall\n");
      printk("$epc = 0x%x\n", regs.epc);
      // _halt(0); 
      break;
    }
    case 9:{ // breakpoint
      tf->epc += 4;
      printk("Break\n");
      printk("$epc = 0x%x\n", regs.epc);
     // _halt(0); 
      break;
    }
    case 10:{ // invalid instruction
      printk("Invalid instruction\n");
      printk("$epc = 0x%x\n", regs.epc);
      _halt(0);
      break;
    }
    case 12:{ // overflow
      printk("Overflow\n");
      printk("$epc = 0x%x\n", regs.epc);
      _halt(0);
      break;
    }
    case 13:{ // trap
      tf->epc += 4;
      ev.event = _EVENT_TRAP;
      break;
    }
    default:printk("exccode = %x\n", exccode);_halt(-1);
  }

  if(H){
    H(ev, &regs);
  }

}
