#ifndef __NPC_H__
#define __NPC_H__

#define EX_ENTRY 0x20
#define SERIAL_PORT ((volatile char *)0x40001000)
#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c
#define GPIO_TRAP ((volatile char *)0x40000000)
#define HZ 50000000
#define INTERVAL 0x10000
#ifndef __ASSEMBLER__

#include <arch.h>

#define cp0_badvaddr 8
#define cp0_count    9
#define cp0_compare  11
#define cp0_status   12
#define cp0_cause    13
#define cp0_epc      14

#define MFC0(dst, src, sel) \
asm volatile("mfc0 %0, $"_STR(src)", %1\n\t":"=r"(dst):"i"(sel))

#define MTC0(dst, src, sel) \
asm volatile("mtc0 %0, $"_STR(dst)", %1\n\t"::"g"(src),"i"(sel))

#define _STR(x) _VAL(x)
#define _VAL(x) #x


struct TrapFrame{
  uint32_t at,
  v0,v1,
  a0,a1,a2,a3,
  t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,
  s0,s1,s2,s3,s4,s5,s6,s7,
  k0,k1,
  gp,sp,fp,ra,
  epc, cause, status, badvaddr;
};

void serial_init();

uint32_t inline GetCount(int sel){
  uint32_t tick = 0;
  if(sel == 0)
    MFC0(tick, cp0_count, 0);
  else if(sel == 1)
    MFC0(tick, cp0_count, 1);
  else
    _halt(1);
  return tick;
}

void inline SetCompare(uint32_t compare){
  MTC0(cp0_compare, compare, 0);
}
#endif

#endif
