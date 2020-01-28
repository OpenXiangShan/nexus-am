#include <am.h>
#include <klib.h>

_Context *on_intr(_Event ev, _Context *ctx) {
  return ctx;
}

void foo() {
  _intr_write(1);
  int n = 0;
  while (1) { 
    n++;
    for (volatile int i = 0; i < 100000; i++);
    _putc("_-=+"[_cpu()]);
    asm volatile("int $0x80");
    if (n == 1000) _halt(34);
  }
}

int main(const char *args) {
  printf("Hello, args = \"%s\"\n", args);
  _cte_init(on_intr);
  _mpe_init(foo);
  return 0;
}
