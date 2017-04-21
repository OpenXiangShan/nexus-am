#include <am.h>
#include <npc.h>

void memory_init(){
	//not to do
	//set _heap
}

void serial_init(){
	//not to do
}

void _halt(int code) {
	_putc('P');
	_putc('a');
	_putc('n');
	_putc('i');
	_putc('c');
	_putc('\n');
	while(1);
}
