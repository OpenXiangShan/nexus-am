#include <am.h>
#include <npc.h>

// TODO: implement these functions.
static void memory_init(){
	//not to do
}

static void serial_init(){
	//not to do
}

void _trm_init() {
	serial_init();
	memory_init();
}

void _ioe_init() {
	vga_init();
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

ulong _uptime() {
  return 0;
}

_Area _heap;
_Screen _screen;
