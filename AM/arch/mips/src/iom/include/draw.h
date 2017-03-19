#ifndef draw_h
#define draw_h

typedef struct {
	char c;
	unsigned int x;
	unsigned int y;
	char color;
} character;

void draw_character(surface *screen,character *ch);
void draw_string(surface *screen,int x,int y,char *str,char color);

#endif
