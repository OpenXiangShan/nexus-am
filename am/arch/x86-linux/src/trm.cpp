#include <am.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

extern "C" {

void _trm_init() {
}

void _putc(char ch) {
  putchar(ch);
}

void _halt(int code) {
  exit(code);
}

_Area _heap;

}
