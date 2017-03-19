#include <am.h>
#include <klib.h>

extern char end;
static ulong heap = (ulong)&end;

// TODO: this lets memory leak!

void* kalloc(size_t size) {
  while (heap % size != 0) heap ++;
  heap += size;
  return (void*)(heap - size);
}

void kfree(void *ptr) {
  assert(0);
}
