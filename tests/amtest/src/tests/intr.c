#include <amtest.h>

_Context *simple_trap(_Event ev, _Context *ctx) {
  switch(ev.event) {
    case _EVENT_IRQ_TIMER:
      printf("t"); break;
    case _EVENT_IRQ_IODEV:
      printf("d"); read_key(); break;
    case _EVENT_YIELD:
      printf("y"); break;
  }
  return ctx;
}

void hello_intr() {
  printf("Hello, AM World @ " __ISA__ "\n");
  printf("  t = timer, d = device, y = yield\n");
  _intr_write(1);
  // printf("hello intr written\n");
  while (1) {
    for (volatile int i = 0; i < 1000; i++) ;
    _yield();
  }
}

void hello_n(int n);
void hello_intr_n(int n) {
  printf("Hello, AM World @ " __ISA__ "\n");
  printf("  t = timer, d = device, y = yield\n");
  _intr_write(1);
  for (volatile int i = 0; i < n; i++) {
    hello_n(i);
    _yield();
  }
}
