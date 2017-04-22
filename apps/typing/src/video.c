#include "video.h"
#include "common.h"

extern _Pixel fb[1024*768];

void prepare_buffer(){
	_draw_f(fb);
}

void display_buffer(){
	_draw_sync();
}
