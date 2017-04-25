#ifndef __NPC_H__
#define __NPC_H__

#define VMEM_ADDR ((void *)0xc0000000)
#define SCR_WIDTH 320
#define SCR_HEIGHT 200
#define SCR_SIZE (SCR_WIDTH * SCR_HEIGHT)
#define KEY_CODE_ADDR ((volatile unsigned int *)0xf0000000)
#define KEY_CODE (*KEY_CODE_ADDR)
#define HEAP_START 0x10000
#define HEAP_END 0x1fffc
static inline u8 R(_Pixel p) { return p >> 16; }
static inline u8 G(_Pixel p) { return p >> 8; }
static inline u8 B(_Pixel p) { return p; }

static inline _Pixel pixel(u8 r, u8 g, u8 b) {
  return (r << 16) | (g << 8) | b;
}
void vga_init();
void serial_init();
void memory_init();

#endif
