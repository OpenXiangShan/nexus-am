#include <am.h>

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  return 0;
}

_Context* _kcontext(_Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void _yield() {
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
