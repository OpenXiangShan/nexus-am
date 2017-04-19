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

void _panic(int code) {
  if (code != 0) {
    printf("Panic (%d)\n", code);
  }
  exit(code);
}

}
