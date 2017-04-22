#include <klib.h>

static unsigned long next = 1;

int rand(){
	next = next * 1103515245 + 12345;
	return ((unsigned)(next/65536) % 32768);
}

void mysrand(unsigned seed){
	next = seed;
}
