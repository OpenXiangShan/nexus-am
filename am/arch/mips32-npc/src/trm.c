#include <am.h>
#include <npc.h>

#define MAX_MEMORY_SIZE 0x4000000

_Area _heap;
_Screen _screen;

extern char _heap_start;
extern char __bss_start;
extern char __bss_end;

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

void memory_init(){
  //probe a memory for heap
  _heap.start = (void *)(&_heap_start);
  _heap.end = (char *)(&_heap_start) + MAX_MEMORY_SIZE;
  volatile char *st = (void *)(&__bss_start);
  volatile char *ed = (void *)(&__bss_end);
  for(; st != ed; st++) {
    *st = 0 ;
  }
}

void serial_init(){
  //not to do
}
