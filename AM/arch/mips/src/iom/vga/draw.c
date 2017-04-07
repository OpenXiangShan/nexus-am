extern char font8x8_basic[128][8];

void draw_character(surface *screen,character *ch){
	int i,j = 0;
	char *p = font8x8_basic[(int)(ch->c)];
	pixel pi;
	pi.color = ch->color;
	for(i = 0; i < 8; i++)
		for(j = 0; j < 8; j++)
			if((p[i] >> j) & 1){
				pi.x = ch->x + i;
				pi.y = ch->y + j;
				draw_pixel(screen,&pi);
			}
}

void draw_string(surface *screen,int x,int y,char *str,char color) {
	character ch;
	ch.color = color;
	while (*str) {
		ch.c = *str++;
		ch.x = x;
		ch.y = y;
		draw_character(screen,&ch);
		if (y + 8 >= screen->width) {
			x += 8; y = 0;
		} 
		else {
			y += 8;
		}
	}
}
	
