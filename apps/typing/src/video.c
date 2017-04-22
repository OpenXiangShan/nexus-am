#include "video.h"
#include "common.h"

//extern _Pixel fb[1024*768];

void prepare_buffer(){
	int i,j;
	for(i = 0; i < _screen.width; i++)
		for(j = 0; j < _screen.height; j++)
			_draw_p(i,j,0x0);
	//_draw_f(fb);
}

void display_buffer(){
	_draw_sync();
}
