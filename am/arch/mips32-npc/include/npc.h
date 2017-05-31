#ifndef __NPC_H__
#define __NPC_H__

#define VMEM_ADDR ((void *)0xc0000000)
#define SCR_WIDTH 200
#define SCR_HEIGHT 200
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)
#define SERIAL_PORT ((void *)0x100000)
#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c
#define HZ 50000000
#define MAX_MEMORY_SIZE 0x4000000
#define INTERVAL 1000

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
void vga_init();
void serial_init();
void memory_init();
u32 GetCount(int sel);
void SetCompare(u32 compare);

#endif
