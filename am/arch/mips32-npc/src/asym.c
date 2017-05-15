#include <am.h>
#include <npc.h>
#include <arch.h>
#define HZ 50
#define INTERVAL (HZ * 1000)
#define INTERVAL_MIN 1000  //min interval(cpu use 1000 cycles to deal interrupt)
#define COUNT_MAX 0xffffffff	//count reg max value

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
#ifdef NOTIME
  return npc_time++;
#else
  return npc_time;
#endif
}

void _time_event(){
  npc_time++;
}

ulong _cycles(){
  return GetCount() / 1000;
}

void _asye_init(){
#ifdef NOTIME
  return;
#else
  u32 count= GetCount();
  if(count + INTERVAL > COUNT_MAX){
    SetCompare(count + INTERVAL - COUNT_MAX);
  }
  else{//count overflow
    SetCompare(count + INTERVAL);
  }
#endif
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
      u32 count= GetCount();
      if(count + INTERVAL > COUNT_MAX){
        SetCompare(count + INTERVAL - COUNT_MAX);
      }
      else{//count overflow
        SetCompare(count + INTERVAL);
      }
    }break;
    default:_halt(0);
  }
}
