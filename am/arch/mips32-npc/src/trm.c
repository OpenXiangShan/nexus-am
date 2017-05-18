#include <am.h>
#include <npc.h>

_Area _heap;
_Screen _screen;
extern int main();

char __attribute__((__noinline__)) get_stat(){
  char *stat = SERIAL_PORT + STAT;
  return *stat;
}

void _trm_init() {
  serial_init();
  memory_init();
  int ret = main();
  _halt(ret);
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
  extern char _end;
  extern char __bss_start;
  unsigned int st = (unsigned int)(&_end);
  unsigned int ed = (unsigned int)(&_end) + MAX_MEMORY_SIZE;
  _heap.start = (void *)(st);
  _heap.end = (void *)(ed);
  st = (unsigned int)(&__bss_start);
  ed = (unsigned int)(&_end);
  char *bss = (void *)(st);
  unsigned int i;
  for(i = st;i < ed; i++) { bss[i - st] = 0; }
}

void serial_init(){
  //not to do
}
