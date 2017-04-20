#ifndef __VGA_H__
#define __VGA_H__

#define VMEM (void *)0xa0000
#define VMEM_SIZE (0xbffff - 0xa0000)
#define WIDTH 320
#define HEIGHT 200

void init_vga();
void draw_pixel(int x,int y,int color);
#endif
