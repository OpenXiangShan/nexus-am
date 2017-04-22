#include <am.h>
#include <npc.h>

extern char font8x8_basic[128][8];

void draw_character(char ch, int x, int y, _Pixel p) {
	int i, j;
<<<<<<< HEAD
	_Pixel c = Getpixel(color);
	char *p = font8x8_basic[(int)ch];
=======
	char *c = font8x8_basic[(int)ch];
>>>>>>> master
	for (i = 0; i < 8; i ++) 
		for (j = 0; j < 8; j ++) 
			if ((c[i] >> j) & 1)
				_draw_p(y + j, x + i, p);
}

void draw_string(const char *str, int x, int y, _Pixel p) {
	while (*str) {
		draw_character(*str ++, x, y, p);
		if (y + 8 >= SCR_WIDTH) {
			x += 8; y = 0;
		} else {
			y += 8;
		}
	}
}
