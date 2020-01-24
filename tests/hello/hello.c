#include <am.h>
#include <klib.h>

int main(const char *args) {
  printf("Hello with args=\"%s\"\n", args);
  printf("Heap: %p, %p, %p\n", _heap.start, _heap.end, -1L);
  return 0;
}
