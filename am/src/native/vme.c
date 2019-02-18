#include <am.h>
#include <stdlib.h>
#include <klib.h>

#define PGSIZE  4096
#define PGSHIFT 12

typedef struct PageMap {
  uintptr_t vpn;
  uintptr_t ppn;
  struct PageMap *next;
  int is_mapped;
} PageMap;

#define list_foreach(p, head) \
  for (p = head; p != NULL; p = p->next)

void shm_mmap(void *va, void *pa, int prot);
void shm_munmap(void *va);

static int vme_enable = 0;

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  // we do not need to ask MM to get a page from OS,
  // since we can call malloc() in native
  vme_enable = 1;
  return 0;
}

int _protect(_AddressSpace *p) {
  p->ptr = NULL;
  p->pgsize = PGSIZE;
  return 0;
}

void _unprotect(_AddressSpace *p) {
}

static _AddressSpace empty_as = { .ptr = NULL };
static _AddressSpace *cur_as = &empty_as;

void get_cur_as(_Context *c) {
  c->prot = cur_as;
}

void _switch(_Context *c) {
  if (!vme_enable) return;

  _AddressSpace *p = c->prot;
  assert(p != NULL);
  if (p == cur_as) return;

  PageMap *pp;
  // munmap all mappings
  list_foreach(pp, cur_as->ptr) {
    if (pp->is_mapped) {
      shm_munmap((void *)(pp->vpn << PGSHIFT));
      pp->is_mapped = false;
    }
  }

  // mmap all mappings
  list_foreach(pp, p->ptr) {
    shm_mmap((void *)(pp->vpn << PGSHIFT), (void *)(pp->ppn << PGSHIFT), 0);
    pp->is_mapped = true;
  }

  cur_as = p;
}

int _map(_AddressSpace *p, void *va, void *pa, int prot) {
  uintptr_t vpn = (uintptr_t)va >> PGSHIFT;
  PageMap *pp;
  list_foreach(pp, p->ptr) {
    // can not remap
    // Actually this is allowed according to the semantics of AM API,
    // but we do this to catch unexcepted behavior from Nanos-lite
    if (pp->vpn == vpn) {
      printf("check remap: %p -> %p, but previously %p -> %p\n", va, pa, pp->vpn << PGSHIFT, pp->ppn << PGSHIFT);
      assert(pp->ppn == ((uintptr_t)pa >> PGSHIFT));
      return 0;
    }
  }

  pp = malloc(sizeof(PageMap));
  pp->vpn = vpn;
  pp->ppn = (uintptr_t)pa >> PGSHIFT;
  pp->next = p->ptr;
  p->ptr = pp;

  if (p == cur_as) {
    // enforce the map immediately
    shm_mmap((void *)(pp->vpn << PGSHIFT), (void *)(pp->ppn << PGSHIFT), 0);
    pp->is_mapped = true;
  }
  else {
    pp->is_mapped = false;
  }
  
  return 0;
}

void get_example_uc(_Context *r);

_Context *_ucontext(_AddressSpace *p, _Area ustack, _Area kstack, void *entry, void *args) {
  ustack.end -= 1 * sizeof(uintptr_t);  // 1 = retaddr
  uintptr_t ret = (uintptr_t)ustack.end;
  *(uintptr_t *)ret = 0;

  _Context *c = (_Context*)ustack.end - 1;
  get_example_uc(c);
  c->rip = (uintptr_t)entry;
  c->prot = p;

  c->uc.uc_mcontext.gregs[REG_RDI] = 0;
  c->uc.uc_mcontext.gregs[REG_RSI] = ret; // ???
  c->uc.uc_mcontext.gregs[REG_RDX] = ret; // ???

  return c;
}
