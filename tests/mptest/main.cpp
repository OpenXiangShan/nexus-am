#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <amtrace.h>

void f() {
  if (_cpu() == 1) {
    _intr_write(1);
  }
  while (1) {
    // printf("%d", _cpu());
  }
}

static uintptr_t start;

static _Context *interrupt(_Event ev, _Context *ctx) {
  printf("Interrupt at core #%d\n", _cpu());
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
  _trace_on(_TRACE_ALL);
  start = (uintptr_t)_heap.start;
  _asye_init(interrupt);
  _pte_init(alloc, free);
  _mpe_init(f);
  assert(0);
  return 0;
}
