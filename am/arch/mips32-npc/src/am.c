#include <am.h>
#include <npc.h>

// TODO: implement these functions.

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

ulong npc_time = 0;

ulong _uptime() {
  return npc_time ++;
}

ulong npc_cycles = 0;
ulong _cycles(){
  return npc_cycles ++;
}

_Area _heap;
_Screen _screen;
