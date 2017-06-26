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

#ifndef __ASSEMBLER__

#include <arch.h>

struct TrapFrame{
  uint32_t at,
  v0,v1,
  a0,a1,a2,a3,
  t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,
  s0,s1,s2,s3,s4,s5,s6,s7,
  k0,k1,
  gp,sp,fp,ra;
};
void serial_init();

#endif

#endif
