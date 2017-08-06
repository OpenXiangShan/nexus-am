#include <am.h>
#include <klib.h>

extern char _end;
extern int main();

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

#define STKSHIFT 17

// the default size of AsyncScratchPadMemory
uint32_t mem_size = 1 << 21;

_Area _heap;

void send_command_tohost(uint64_t device, uint64_t cmd, uint64_t payload)
{
  uint64_t command = (device << 56) | (cmd << 56 >> 8) | (payload << 16 >> 16);
  while(tohost != 0);
  tohost = command;
}

void _putc(char ch) {
    // device 1 is bcd
    // command 1 is write
    send_command_tohost(1, 1, (uint8_t)ch);
}

void _printstr(char *str) {
  while(str)
    _putc(*str++);
}

void _halt(int code) {
  send_command_tohost(0, 0, (uint32_t)code << 1 | 1);
  while(1);
}

void _trm_init() {
  _heap.start = (void *)(((uintptr_t)&_end + 63) & -64) + (2 << STKSHIFT);
  _heap.end = (void *)0x80000000 + mem_size;
  int ret = main();
  _halt(ret);
}
