#include <riscv32.h>
#include <klib.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static void* (*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  return 0;
}

int _protect(_Protect *p) {
  p->ptr = (PDE*)(pgalloc_usr(1));
  p->pgsize = 4096;

  return 0;
}

void _unprotect(_Protect *p) {
}

static _Protect *cur_as = NULL;
void get_cur_as(_Context *c) {
  c->prot = cur_as;
}

void _switch(_Context *c) {
  cur_as = c->prot;
}

int _map(_Protect *p, void *va, void *pa, int mode) {
  PDE *pt = (PDE*)p->ptr;
  PDE *pde = &pt[PDX(va)];
  if (!(*pde & PTE_V)) {
    *pde = PTE_V | (uint32_t)pgalloc_usr(1);
  }
  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  if (!(*pte & PTE_V)) {
    assert(0);
    //*pte = PTE_V | PTE_D | (((uint32_t)pa >> PGSHFT) << 6);
  }

  return 0;
}

_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {
  _Context *c = (_Context*)ustack.end - 1;

  c->prot = p;
  c->epc = (uintptr_t)entry;
  c->status = 0x3;
  return c;
}
