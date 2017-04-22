#include "common.h"
#include "video.h"

extern char font8x8_basic[128][8];
static inline unsigned int pixel(int color){
	char r = (color >> 24) & 0xff;
	char g = (color >> 16) & 0xff;
	char b = (color >> 8) & 0xff;
	return (r << 16 | g << 8 | b );
}

static char inline *reverse(char *s)  
{  
    char temp;  
    char *p = s;
    char *q = s; 
    while(*q)  
        ++q;  
    q--;  
    while(q > p)  
    {  
        temp = *p;  
        *p++ = *q;  
        *q-- = temp;  
    }  
    return s;  
}  

char *itoa(int n)  
{  
    int i = 0,isNegative = 0;  
    static char s[100];
    if((isNegative = n) < 0)
    {  
        n = -n;  
    }  
    do 
    {  
        s[i++] = n%10 + '0';  
        n = n/10;  
    }while(n > 0);  
  
    if(isNegative < 0) 
    {  
        s[i++] = '-';  
    }  
    s[i] = '\0'; 
    return reverse(s);    
}  

static inline void draw_character(char ch, int x, int y, int color) {
	int i, j;
	char *p = font8x8_basic[(int)ch];
	for (i = 0; i < 8; i ++) 
		for (j = 0; j < 8; j ++) 
			if ((p[i] >> j) & 1)
				_draw_p(y + j, x + i, pixel(color));
}

static inline void draw_string(const char *str, int x, int y, int color) {
	while (*str) {
		draw_character(*str ++, x, y, color);
		if (y + 8 >= SCR_WIDTH) {
			x += 8; y = 0;
		} else {
			y += 8;
		}
	}
}

void
redraw_screen() {
	fly_t it;
	const char *hit, *miss;

	prepare_buffer();
	/* 绘制每个字符 */
	for (it = characters(); it != NULL; it = it->_next) {
		static char buf[2];
		buf[0] = it->text + 'A'; buf[1] = 0;
		draw_string(buf, it->x, it->y, 0xff000000);
	}

	/* 绘制命中数、miss数、最后一次按键扫描码和fps */
	const char *key = itoa(last_key_code());
	draw_string(key, SCR_HEIGHT - 8, 0, 0x0000ff00);
	hit = itoa(get_hit());
	draw_string(hit, 0, SCR_WIDTH - strlen(hit) * 8, 0x0000ff00);
	miss = itoa(get_miss());
	draw_string(miss, SCR_HEIGHT - 8, SCR_WIDTH - strlen(miss) * 8, 0x0000ff00);
	const char *fps = itoa(get_fps());
	draw_string(fps, 0, 0, 0x00ff0000);
	draw_string("FPS", 0, strlen(fps) * 8, 0x00ff0000);
}
