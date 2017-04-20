#include <draw.h>
#include <arch.h>

extern char font8x8_basic[128][8];
void draw_pixel(int x,int y,char color){
	char *vmem = VMEM;
	vmem[x * WIDTH + y] = color;
}

void draw_character(char ch, int x, int y, int color){
	int i, j;
	char *p = font8x8_basic[(int)ch];
	for (i = 0; i < 8; i ++) 
		for (j = 0; j < 8; j ++) 
			if ((p[i] >> j) & 1)
				draw_pixel(x + i, y + j, color);
}

void draw_string(const char *str, int x, int y, int color){
		while (*str) {
		draw_character(*str ++, x, y, color);
		if (y + 8 >= 640) {
			x += 8; y = 0;
		} else {
			y += 8;
		}
	}
}
