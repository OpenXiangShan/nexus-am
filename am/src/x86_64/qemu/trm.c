#include <am.h>

_Area _heap = {}; // the heap memory defined in AM spec

int foo[4096] = {
  1, 2, 3, 4, 5
};

void _start(const char *args) {
  while (1);
}

void _putc(char ch) {
}

void _halt(int code) {
  while (1);
}

void some_code() {
  
}