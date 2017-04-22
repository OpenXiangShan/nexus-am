#include <am.h>
#include <npc.h>

int curr_line = 0;
int curr_col = 0;

<<<<<<< HEAD
extern void draw_character(char ch, int x, int y, int color);
u8 *vmem = VMEM_ADDR;
=======
extern void draw_character(char ch, int x, int y, _Pixel p);

struct FBPixel {
	u8 r : 2;
	u8 g : 4;
	u8 b : 2;
} *fb;
>>>>>>> master

void vga_init(){
	_screen.width = SCR_WIDTH;
	_screen.height = SCR_HEIGHT;
	fb = VMEM_ADDR;
}

void _putc(char ch) {
	//TODO:use uart(now use vga)
	if(ch == '\n'){
		curr_col = 0;
		curr_line += 8;
	}
	else{
		draw_character(ch,curr_line,curr_col,pixel(0xff,0xff,0xff));
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

void _draw_f(_Pixel *p) {//npc doesn't support
	int i;
	for(i = 0;i < SCR_SIZE; i++){
	    	fb[i].r = R(p[i]);
    		fb[i].g = G(p[i]);
    		fb[i].b = B(p[i]);
	}
}

void _draw_p(int x, int y, _Pixel p) {
<<<<<<< HEAD
	vmem[x * SCR_WIDTH + y] = Getcolor(p);
=======
  	fb[x + y * _screen.width].r = R(p);
  	fb[x + y * _screen.width].g = G(p);
  	fb[x + y * _screen.width].b = B(p);
>>>>>>> master
}

void _draw_sync() {
	//not to do
}

int _peek_key(){
	return 0;
}
