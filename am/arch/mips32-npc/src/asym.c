#include <am.h>
#include <npc.h>
#include <arch.h>

u32 GetCount(int sel){
  u32 tick = 0;
  if(sel == 1)
    asm volatile("mfc0 %0, $9, %1\n\t":"=r"(tick):"i"(1));
  else if(sel == 0)
    asm volatile("mfc0 %0, $9, %1\n\t":"=r"(tick):"i"(0));
  else
    _halt(1);
  return tick;
}

void SetCompare(u32 compare){
  asm volatile("mtc0 %0, $11\n\t"::"r"(compare));
}

void _time_event(){
}

void _asye_init(){
  return;
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
  asm volatile("mfc0 %0,$12	\n\t":"=r"(status));
  status = status | 0x2;
  asm volatile("mtc0 %0,$12	\n\t"::"r"(status));
}

void _ienable(){
  int status = 0;
  asm volatile("mfc0 %0,$12	\n\t":"=r"(status));
  status = status & 0xfffffffd;
  asm volatile("mtc0 %0,$12	\n\t"::"r"(status));
}

int _istatus(){
  int status = 0;
  asm volatile("mfc0 %0,$12	\n\t":"=r"(status));
  if((status & 0x2) >> 1){
    return 0;
  }
  else{
    return 1;
  }
}

void irq_handle(struct TrapFrame *tf){
//TODO:handle interrupt
  u32 arg = 0;
  u32 intr = 0;
  asm volatile("add %0,$k1,$zero\n\t":"=r"(arg));
  asm volatile("add %0,$k0,$zero\n\t":"=r"(intr));
  tf = (void *)arg;
  switch(intr){
    case 0x7://time interrupt
    {
      _time_event();
      u32 count= GetCount(0);
      SetCompare(count + INTERVAL);
    }break;
    default:_halt(0);
  }
}
