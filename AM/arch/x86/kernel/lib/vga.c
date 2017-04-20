#include "x86.h"
#include "device.h"

uint16_t *vmem = (void *)0xa0000;
void init_vga(){
}

void draw_pixel(int x,int y,int color){
	vmem[x * WIDTH + y] = 0x0;
}
