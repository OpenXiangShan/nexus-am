#include <am.h>

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  return 0;
}

void _protect(_AddressSpace *as) {
}

void _unprotect(_AddressSpace *as) {
}

void _map(_AddressSpace *as, void *va, void *pa, int prot) {
}

_Context* _ucontext(_AddressSpace *as, _Area kstack, void *entry) {
  return NULL;
}
