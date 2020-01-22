#include <am.h>
#include <x86.h>

_Area _heap = {}; // the heap memory defined in AM spec

int main(const char *args);

void _start_c(char *args) {
  // TODO: build full page table
  int ret = main(args);
  _halt(ret);
}

void _putc(char ch) {
  #define COM1 0x3f8
  outb(COM1, ch);
}

void _halt(int code) {
  while (1);
}

void some_code() {
  
}
