#include <am.h>
#include <npc.h>

void memory_init(){
//0-7fff for code(32k)
//8000-ffff for data(32k)
//10000-17fff for stack(32k)
//18000-1ffff for heap(32k)
	//probe a memory for heap
	_heap.start = (void *)0x18000;
	_heap.end = (void *)0x1ffff;
}

void serial_init(){
	//not to do
}
