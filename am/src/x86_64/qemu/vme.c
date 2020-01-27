#include "x86_64-qemu.h"

int _vme_init(void *(*pgalloc)(size_t size), void (*pgfree)(void *)) { return 0; }
int _protect(_AddressSpace *as) { return 0; }
void _unprotect(_AddressSpace *as) { }
int _map(_AddressSpace *as, void *va, void *pa, int prot) { return 0; }
_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack,
                                 void *entry, void *args) { return NULL; }