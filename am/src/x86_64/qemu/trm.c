#include <am.h>
#include <x86.h>

_Area _heap = {}; // the heap memory defined in AM spec

int main(const char *args);

void _start64(char *args) {
  int ret = main(args);
  _halt(ret);
}

void _putc(char ch) {
}

void _halt(int code) {
  while (1);
}

void some_code() {
  
}