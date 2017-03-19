#ifndef vga_h
#define vga_h

typedef struct {
	unsigned int w;
	unsigned int h;
	char color;
} rect;

typedef struct {
	unsigned int x;
	unsigned int y;
	char color;
} pixel;

typedef struct {
	unsigned int width;
	unsigned int height;
	pixel *pixels;
} surface;

void draw_pixel(surface *screen,pixel *pixels);
void clear_screen(surface *screen,char color);
void copy_rect(surface *screen,unsigned x,unsigned y,rect src);
void flip_screen(surface *screen);

#endif
