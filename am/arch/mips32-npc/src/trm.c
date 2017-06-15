#include <am.h>
#include <npc.h>
#include <klib.h>

int main();

void _trm_init() {
  int ret = main();
  _halt(ret);
}

// -------------------- memory --------------------

extern unsigned int heap_start, _heap_end; // symbols

_Area _heap = {
  .start = &_heap_start,
  .end = &_heap_end,
};

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

