#include <time.h>

int getCount(){
	int tick = 0;
	asm volatile("mfc0 %0, $9\n\t":"=r"(tick));
	return tick;
}

void setCompare(int count){
	asm volatile("mtc0 $11, %0\n\t"::"r"(count));
}
