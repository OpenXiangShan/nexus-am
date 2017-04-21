#include <am.h>
#include <npc.h>

u32 *vmem = VMEM_ADDR;
void vga_init(){
	_screen.width = SCR_WIDTH;
	_screen.height = SCR_HEIGHT;
	int i;
	for(i = 0; i < SCR_SIZE; i++){
		vmem[i] = 0;
	}
}

void _putc(char ch) {
	//TODO:use uart
}

void _draw_f(_Pixel *p) {
	int i;
	for(i = 0;i < SCR_SIZE / 4; i++){
		vmem[i] = Getcolor(p[i]);
	}
}

void _draw_p(int x, int y, _Pixel p) {
	vmem[x * SCR_WIDTH + y] = Getcolor(p);
}

void _draw_sync() {
	//not to do
}
