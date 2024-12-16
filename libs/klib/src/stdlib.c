#include "klib.h"
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

static struct {
  void *ptr;
  uintptr_t size;
} last = { .ptr = NULL, .size = 0 };

volatile uint64_t malloc_lock = 0;

void *malloc(size_t size) {
  lock(&malloc_lock);
  if (last.ptr == NULL) {
    last.ptr = _heap.start;
    printf("heap start = %x\n", last.ptr);
  }

  // aligning
  size = ROUNDUP(size, sizeof(uintptr_t));

  // skip the region allocated by the last call
  last.ptr += last.size;
  if (last.ptr + size >= _heap.end) {
    release(&malloc_lock);
    return NULL;
  }
  void *ret = last.ptr;
  last.size = size;
  release(&malloc_lock);
  return ret;
}

void free(void *ptr) {
  lock(&malloc_lock);
  if (ptr == last.ptr) last.size = 0;
  release(&malloc_lock);
}

#endif
