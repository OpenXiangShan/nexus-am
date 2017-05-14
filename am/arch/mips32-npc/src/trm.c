#include <am.h>
#include <npc.h>

_Area _heap;
_Screen _screen;

extern void *_heap_start;
extern void *_heap_end;
extern char *__bss_start;
extern char *__bss_end;

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
  _heap.start = _heap_start;
  _heap.end = _heap_end;
  char *st = __bss_start;
  char *ed = __bss_end;
  for(;st != ed; st++)
    *st = 0;
}

void serial_init(){
  //not to do
}
