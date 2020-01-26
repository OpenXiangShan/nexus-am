#include <am.h>
#include <klib.h>

void hello() {
  printf("CPU %d: hello!\n", _cpu());
  while (1);
}

_Context *on_intr(_Event ev, _Context *ctx) {
  return ctx;
}

void foo() {
  _intr_write(1);
  int cpu = _cpu();
  while (1) { 
    for (volatile int i = 0; i < 100000; i++);
    _putc("._"[cpu]);
//    asm volatile("int $0x80");
  }
}

int main(const char *args) {
  printf("CPU0: hello with args=\"%s\"\n", args);
  _cte_init(on_intr);
  printf("MPE init\n");
  _mpe_init(foo);
  return 0;
}
