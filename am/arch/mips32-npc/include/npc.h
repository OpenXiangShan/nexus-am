#ifndef __NPC_H__
#define __NPC_H__

#define VMEM_ADDR ((u32 *)0xc0000000)
#define SCR_WIDTH 640
#define SCR_HEIGHT 480
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)

static inline u8 R(_Pixel p) { return p >> 16; }
static inline u8 G(_Pixel p) { return p >> 8; }
static inline u8 B(_Pixel p) { return p; }
void vga_init();
static inline u32 Getcolor(_Pixel p){
	return (R(p) << 16 | G(p) | B(p) << 4);
}

#define KEY_CODE_ADDR ((volatile unsigned int *)0xf0000000)
#define KEY_CODE (*KEY_CODE_ADDR)

#endif
