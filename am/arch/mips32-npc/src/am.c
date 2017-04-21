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

void _asye_init(){
	SetCompare(100);
}

_Area _heap;
_Screen _screen;
