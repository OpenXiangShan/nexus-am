#include "time.h"
#include "common.h"

extern volatile unsigned int tick;
bool IF = 1;

void wait_intr(){
	int now = _uptime();
	while(1){
		if(IF){
			tick = _uptime();
			if((now < tick) || (keyboard_event())){
				break;
			}
		}
	}
}

void cli(){
   	IF = 0;
}

void sti(){
   	IF = 1;
}
