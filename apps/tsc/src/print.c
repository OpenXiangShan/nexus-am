#include <hello.h>
#include <am.h>

void print(const char *s) {
  for (; *s; s ++) {
    _putc(*s);
  }
}

static void print_i(int i) {
  if (i > 0) {
    print_i(i / 10);
    _putc('0' + (i % 10));
  }
}

void printi(int i) {
  if (i == 0) _putc('0');
  else print_i(i);
}


