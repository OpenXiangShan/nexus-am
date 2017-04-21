#include "video.h"
#include "common.h"

_Pixel fb[640 *480] = {0};

void prepare_buffer(){
	_draw_f(fb);
}

void display_buffer(){
	_draw_sync();
}
