#include "vga.h"

char *vmem = NULL;

void init_vga(){
	vmem = VMEM_ADDR;
}
void draw_pixel(surface *screen,pixel *pixels){
//draw a pixel(8 bits) on (x,y) pos of surface
	if(pixels->x > screen->width || pixels->y > screen->height){
	//cause vmem overflow
		return;
	}
	screen->pixels[pixels->x * screen->width + pixels->y].color = pixels->color;
}

void clear_screen(surface *screen,char color){
//clear surface into one color
	unsigned int i;
	for(i = 0; i < (screen->width) * (screen->height); i++)
		screen->pixels[i].color = color;
}

void copy_rect(surface *screen,unsigned x,unsigned y,rect src){
//copy a rectangle to (x,y) pos of surface
	unsigned int i,j;
	pixel p;
	p.color = src.color;
	for(i = 0;i < src.w;i++){
		for(j = 0; j < src.h;j++){
			//screen->pixels[(x + i) * (screen->width) + (y + j)].color = src.color;
			p.x = x + i;
			p.y =y + j;
			draw_pixel(screen,&p);
		}
	}
}

void flip_screen(surface *screen){
//flip the real screen by a colored surface
	unsigned int i;
	for(i = 0; i < (screen->width) * (screen->height); i++){
		vmem[i] = screen->pixels[i].color;
	}
}
