#include <am.h>
#include <npc.h>

#define MAX_MEMORY_SIZE 0x4000000

_Area _heap;
_Screen _screen;

extern unsigned int _heap_start;
extern unsigned int __bss_start;
extern unsigned int __bss_end;

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
  volatile unsigned int st = _heap_start;
  volatile unsigned int ed = _heap_start + MAX_MEMORY_SIZE;
  _heap.start = (void *)st;
  _heap.end = (void *)ed;
  st = __bss_start;
  ed = __bss_end;
  volatile char *bss = (void *)st;
  volatile unsigned int i;
  for(i = st; i < ed; i++) {
    bss[i - st] = 0 ;
  }
}

void serial_init(){
  //not to do
}
