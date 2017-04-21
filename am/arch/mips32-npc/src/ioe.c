#include <am.h>
#include <npc.h>

int curr_line = 0;
int curr_col = 0;

extern void draw_character(char ch, int x, int y, int color);

u32 *vmem = VMEM_ADDR;
void vga_init(){
	_screen.width = SCR_WIDTH;
	_screen.height = SCR_HEIGHT;
}

void _putc(char ch) {
	//TODO:use uart
	if(ch == '\n'){
		curr_col = 0;
		curr_line += 8;
	}
	else{
		draw_character(ch,curr_line,curr_col,0x3c);
	}
	if (curr_col + 8 >= SCR_WIDTH) {
			curr_line += 8; curr_col = 0;
	} else {
			curr_col += 8;
	}
	if(curr_line >= SCR_HEIGHT){
		curr_line = 0;
	}
}

void _draw_f(_Pixel *p) {
	int i;
	for(i = 0;i < SCR_SIZE; i++){
		vmem[i] = Getcolor(p[i]);
	}
}

void _draw_p(int x, int y, _Pixel p) {
	vmem[x * SCR_WIDTH + y] = p;
}

void _draw_sync() {
	//not to do
}
