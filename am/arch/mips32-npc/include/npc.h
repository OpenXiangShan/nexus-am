#ifndef __NPC_H__
#define __NPC_H__

#define VMEM_ADDR ((void *)0x40010000)
#define SCR_WIDTH 320
#define SCR_HEIGHT 200
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)
#define SERIAL_PORT ((char *)0x40001000)
#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c
#define GPIO_TRAP ((char *)0x40000000)
#define HZ 500000
#define MAX_MEMORY_SIZE 0x4000000
#define INTERVAL 1000
#define TIMER_BASE ((volatile char *)0x41c00000)
   
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


static inline u8 R(_Pixel p) { return p >> 16; }
static inline u8 G(_Pixel p) { return p >> 8; }
static inline u8 B(_Pixel p) { return p; }

static inline _Pixel pixel(u8 r, u8 g, u8 b) {
  return (r << 16) | (g << 8) | b;
}

struct TrapFrame{
  u32 at,
  v0,v1,
  a0,a1,a2,a3,
  t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,
  s0,s1,s2,s3,s4,s5,s6,s7,
  k0,k1,
  gp,sp,fp,ra;
};

u32 GetCount(int sel);
void SetCompare(u32 compare);

char in_byte();
void out_byte(char);


void timer_init();
u32 get_TCR();

#endif
