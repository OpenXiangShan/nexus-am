#include <am.h>
#include <npc.h>

extern char _end;

_Area _heap = {
  .start = &_end,
  .end   = &_end,
};

_Screen _screen = {
  .width  = 0,
  .height = 0,
};

static char uart_stat() {
  return SERIAL_PORT[STAT];
}

static void uart_send(char ch) {
  while((uart_stat() >> 3) & 0x1);
  SERIAL_PORT[Tx] = ch;
}

char uart_recv() {
  if(!(uart_stat() & 0x1)) return '\0';
  else return SERIAL_PORT[Rx];
}

void _putc(char ch) {
  if(ch == '\n') {
    uart_send('\r');
  }
  uart_send(ch);
}

void _halt(int code) {
  for (const char *p = "Halt\n"; *p; p ++) {
    _putc(*p);
  }
  GPIO_TRAP[0] = code;

  while (1);
}
