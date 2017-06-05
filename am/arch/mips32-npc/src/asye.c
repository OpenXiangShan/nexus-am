#include <am.h>
#include <npc.h>
#include <arch.h>
#include <klib.h>

// TODO: why this is in the asye?
u32 GetCount(int sel){
  u32 tick = 0;
  if(sel == 0)
    MFC0(tick, cp0_count, 0);
  else if(sel == 1)
    MFC0(tick, cp0_count, 1);
  else
    _halt(1);
  return tick;
}

void SetCompare(u32 compare){
  MTC0(cp0_compare, compare, 0);
}

void _time_event(){
}

void _asye_init(){
}

void _listen(_RegSet* (*l)(_Event ex, _RegSet *regs)){
}

_RegSet *_make(_Area kstack, void *entry){
  _RegSet *r = (void *)0;
  return r;
}

void _trap(){
}

void _idle(){
}

void _idisable(){
  int status = 0;
  MFC0(status, cp0_status, 0);
  status = status | 0x2;
  MTC0(cp0_status, status, 0);  
}

void _ienable(){
  int status = 0;
  MFC0(status, cp0_status, 0);
  status = status & 0xfffffffd;
  MTC0(cp0_status, status, 0);  
}

int _istatus(){
  int status = 0;
  MFC0(status, cp0_status, 0);
  if((status & 0x2) >> 1){
    return 0;
  }
  else{
    return 1;
  }
}

void irq_handle(struct TrapFrame *tf){
//TODO:handle interrupt
  u32 arg = 0, IPExcCode = 0;
  u8  IPCode = 0, ExcCode = 0;
  u32 EPC = 0, BadVaddr = 0;

  // cp0 info & general regfiles, values of k0 & k1 come from trap.S
  asm volatile("addu %0,$k1,$zero\n\t":"=r"(arg));
  asm volatile("addu %0,$k0,$zero\n\t":"=r"(IPExcCode));
  MFC0(EPC, cp0_epc, 0);
  MFC0(BadVaddr, cp0_badvaddr, 0);
  
  tf = (void *)arg;
  IPCode = (IPExcCode & 0xff00) >> 8;
  ExcCode = (IPExcCode & 0xff) >> 2;
  

  //TODO: exception handling
  // Delayslot should be considered when handle exceptions !!!
  switch(ExcCode){
    case 0:{ //interrupt
      switch(IPCode){
      	case 0x80: // time interrupt
      	{
            _time_event();
            u32 count= GetCount(0);
            SetCompare(count + INTERVAL);
      	}
      	break;
      	default:_halt(0);
      }
    }
    break;
    case 4:{ //AdEL
      printk("AdEL\n");
      printk("$epc = 0x%x\n", EPC);
      printk("$BadVaddr = 0x%x\n", BadVaddr);
      _halt(0);
    }
    break;
    case 5:{ //AdES
      printk("AdES\n");
      printk("$epc = 0x%x\n", EPC);
      printk("$BadVaddr = 0x%x\n", BadVaddr);
      _halt(0);
    }
    break;
    case 8:{ // syscall
      printk("Syscall\n");
      printk("$epc = 0x%x\n", EPC);
      // epc + 4 -> used to eret to next instruction, instead of syscall
      EPC += 4;
      MTC0(cp0_epc, EPC, 0);
      _halt(0);
    }
    break;
    case 9:{ // breakpoint
      printk("Break\n");
      printk("$epc = 0x%x\n", EPC);
     _halt(0); 
    }
    break;
    case 10:{ // invalid instruction
      printk("Invalid instruction\n");
      printk("$epc = 0x%x\n", EPC);
      _halt(0);
    }
    break;
    case 12:{ // overflow
      printk("Overflow\n");
      printk("$epc = 0x%x\n", EPC);
      _halt(0);
    }
    break;
    case 13:{ // trap
      printk("Trap\n");
      printk("$epc = 0x%x\n", EPC);
      EPC += 4;
      MTC0(cp0_epc, EPC, 0);
      _halt(0);
    }
    break;
    default:_halt(0);
  }
}
