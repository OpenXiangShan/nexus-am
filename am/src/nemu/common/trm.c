#include <am.h>
#include <nemu.h>
#include <klib-macros.h>

// common part of TMR

extern char _heap_start;
int main(const char *args);

_Area _heap = RANGE(&_heap_start, &_pmem_end);

void _trm_init() {
  extern const char __am_mainargs;
  int ret = main(&__am_mainargs);
  _halt(ret);
}

// these APIs are defined under the isa-dependent directory

void _putc(char ch);
void _halt(int code);
