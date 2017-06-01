#include <am.h>
#include <npc.h>
#include <klib.h>

extern int main();
extern unsigned int _bss_start, _bss_end, _heap_start, _heap_end; // symbols

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};
_Screen _screen;

char __attribute__((__noinline__)) get_stat(){
  char *stat = SERIAL_PORT + STAT;
  return *stat;
}

void _trm_init() {
  // clean up bss
  for (u32 *p = &_bss_start; p != &_bss_end; p ++) {
    *p = 0;
  }
  int ret = main();
  _halt(ret);
}

void _ioe_init() {
  vga_init();
}

void _halt(int code) {
  printf("Exited (%d)\n", code);
  GPIO_TRAP[0] = code;
  while(1);
}

