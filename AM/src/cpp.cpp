#include <am.h>
#include <klib.h>

// C++ dependent features

void *operator new(size_t size) {
  return kalloc(size);
}

void *operator new[](size_t size) {
  return kalloc(size);
}

void operator delete(void *ptr) {
  kfree(ptr);
}

void operator delete[](void *ptr) {
  kfree(ptr);
}

