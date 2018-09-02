#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <amtrace.h>

void f() {
  _intr_write(1);
  if (_cpu() == 1) {
    for (int volatile i = 0; i < 10000000; i++) ;
    assert(0);
  }
  while (1) {
    printf("%d", _cpu());
  }
}

static uintptr_t start;

static _Context *interrupt(_Event ev, _Context *ctx) {
  return ctx;
}

static void *alloc(size_t size) {
  while (start % size != 0) start++;
  void *ret = (void *)start;
  start += size;
  return ret;
}

static void free(void *ptr) {
}

int main() {
  start = (uintptr_t)_heap.start;
  _trace_on(_TRACE_ALL);
  _cte_init(interrupt);
  _vme_init(alloc, free);
  _mpe_init(f);
  assert(0);
  return 0;
}
