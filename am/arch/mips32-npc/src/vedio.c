#include <am.h>
#include <npc.h>

extern char font8x8_basic[128][8];

void draw_character(char ch, int x, int y, int color) {
	int i, j;
	_Pixel c = Getpixel(color);
	char *p = font8x8_basic[(int)ch];
	for (i = 0; i < 8; i ++) 
		for (j = 0; j < 8; j ++) 
			if ((p[i] >> j) & 1)
				_draw_p(x + i, y + j, c);
}

void draw_string(const char *str, int x, int y, int color) {
	while (*str) {
		draw_character(*str ++, x, y, color);
		if (y + 8 >= SCR_WIDTH) {
			x += 8; y = 0;
		} else {
			y += 8;
		}
	}
}
