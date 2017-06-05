#include <am.h>
#include <npc.h>
#include <klib.h>

int main();
static void memory_init();

void _trm_init() {
  memory_init();
  int ret = main();
  _halt(ret);
}

// -------------------- memory --------------------

extern unsigned int _bss_start, _bss_end, _heap_start, _heap_end; // symbols

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

static void memory_init() {
  // TODO: This should be done by the loader.
  // When the loader is fixed, delte:
  //   1. this function
  //   2. symbols in loader.ld
  for (u32 *p = &_bss_start; p != &_bss_end; p ++) {
    *p = 0;
  }
}

// -------------------- debug console --------------------

volatile static char *csend = SERIAL_PORT + Tx;
volatile static char *crecv = SERIAL_PORT + Rx;

static char __attribute__((__noinline__)) get_stat(){
  volatile char *stat = SERIAL_PORT + STAT;
  return *stat;
}

void out_byte(char ch) {
  while((get_stat() >> 3) & 0x1);
  *csend = ch;
}

char in_byte() {
  if(!(get_stat() & 0x1)) return '\0';
  else return *crecv;
}

void _putc(char ch) {
  if(ch == '\n') {
    out_byte('\r');
  }
  out_byte(ch);
}


// -------------------- halting --------------------

void _halt(int code) {
  const char *msg = (code == 0) ?
    "Exited (0)." :
    "Exited (error).";

  for (; *msg; msg ++) {
    _putc(*msg);
  }

  GPIO_TRAP[0] = code;
  while(1);
}

