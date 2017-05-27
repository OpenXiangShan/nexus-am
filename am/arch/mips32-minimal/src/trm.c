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
  uart_send(ch);
}

void _halt(int code) {
  _putc('P');
  _putc('a');
  _putc('n');
  _putc('i');
  _putc('c');
  _putc('\n');

  GPIO_TRAP[0] = code;

  while (1);
}
