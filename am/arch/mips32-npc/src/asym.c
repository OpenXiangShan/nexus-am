#include <am.h>
#include <npc.h>
#include <arch.h>

ulong npc_cycles = 0;
ulong npc_time = 1;

u32 GetCount(){
  u32 tick = 0;
  asm volatile("mfc0 %0, $9\n\t":"=r"(tick));
  return tick;
}

void SetCompare(u32 compare){
  asm volatile("mtc0 %0, $11\n\t"::"r"(compare));
}

ulong _uptime() {
  return npc_time;
}

void _time_event(){
  npc_time++;
}

ulong _cycles(){
  return 0;
}

void _asye_init(){
  u32 count= GetCount();
  SetCompare(count + 20000);
}

void _listen(_RegSet* (*l)(int ex, _RegSet *regs)){
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
}

void _ienable(){
}

int _istatus(){
	return 0;
}

void irq_handle(struct TrapFrame *tf){
//TODO:handle
  u32 arg = 0;
  asm volatile("add %0,$k1,$zero\n\t":"=r"(arg));
  tf = (void *)arg;
  u32 intr = 0;
  asm volatile("add %0,$k0,$zero\n\t":"=r"(intr));
  switch(intr){
    case 0x7://time interrupt
    {
      _time_event();
      u32 count= GetCount();
      if(count + 20000 > 2147483647){
        SetCompare(count + 20000 - 2147483647);
      }
      else{
        SetCompare(count + 20000);
      }
    }break;
    default:_putc('0' + intr);
  }
  asm volatile("nop");
}
