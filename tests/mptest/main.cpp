#include <am.h>
#include <amdev.h>
#include <klib.h>

void f() {
  while (1) {
    printf("%d", _cpu());
  }
}

static uintptr_t start;

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
  _pte_init(alloc, free);
  _mpe_init(f);
  assert(0);
  return 0;
}
