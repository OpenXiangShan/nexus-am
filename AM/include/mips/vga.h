#ifndef __VGA_H__
#define __VGA_H__

void draw_pixel(int x,int y,char color);
void prepare_buffer();
void draw_character(char ch, int x, int y, int color);
void draw_string(const char *str, int x, int y, int color);
#endif
