#include <am.h>
#include <npc.h>

// TODO: implement these functions.

void serial_init(){
	//not to do
}

void memory_init(){
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
  while(1);
}

ulong _uptime() {
  ulong tick = 0;
  asm volatile("mfc0 %0, $9\n\t":"=r"(tick));
  return tick;
}

ulong _cycles(){
  return 0;
}
_Area _heap;
_Screen _screen;
