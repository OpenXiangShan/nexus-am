#include "platform.h"

#define USER_SPACE RANGE(0x40000000, 0xc0000000)

typedef struct PageMap {
  void *va;
  void *pa;
  struct PageMap *next;
  int prot;
  int is_mapped;
} PageMap;

#define list_foreach(p, head) \
  for (p = head; p != NULL; p = p->next)

extern int __am_pgsize;
static int vme_enable = 0;
static void* (*pgalloc)(size_t) = NULL;
static void (*pgfree)(void *) = NULL;

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc = pgalloc_f;
  pgfree = pgfree_f;
  vme_enable = 1;
  return 0;
}

void _protect(_AddressSpace *as) {
  assert(as != NULL);
  as->ptr = NULL;
  as->pgsize = __am_pgsize;
  as->area = USER_SPACE;
}

void _unprotect(_AddressSpace *as) {
}

void __am_switch(_Context *c) {
  if (!vme_enable) return;

  _AddressSpace *as = c->as;
  if (as == thiscpu->cur_as) return;

  PageMap *pp;
  if (thiscpu->cur_as != NULL) {
    // munmap all mappings
    list_foreach(pp, thiscpu->cur_as->ptr) {
      if (pp->is_mapped) {
        __am_shm_munmap(pp->va);
        pp->is_mapped = false;
      }
    }
  }

  if (as != NULL) {
    // mmap all mappings
    list_foreach(pp, as->ptr) {
      assert(IN_RANGE(pp->va, USER_SPACE));
      __am_shm_mmap(pp->va, pp->pa, pp->prot);
      pp->is_mapped = true;
    }
  }

  thiscpu->cur_as = as;
}

void _map(_AddressSpace *as, void *va, void *pa, int prot) {
  assert(IN_RANGE(va, USER_SPACE));
  assert((uintptr_t)va % __am_pgsize == 0);
  assert((uintptr_t)pa % __am_pgsize == 0);
  assert(as != NULL);
  PageMap *pp;
  list_foreach(pp, as->ptr) {
    // can not remap
    // Actually this is allowed according to the semantics of AM API,
    // but we do this to catch unexcepted behavior from Nanos-lite
    if (pp->va == va) {
      printf("check remap: %p -> %p, but previously %p -> %p\n", va, pa, pp->va, pp->pa);
      assert(pp->pa == pa);
      return;
    }
  }

  pp = pgalloc(__am_pgsize); // this will waste memory, any better idea?
  pp->va = va;
  pp->pa = pa;
  pp->prot = prot;
  pp->next = as->ptr;
  as->ptr = pp;

  if (as == thiscpu->cur_as) {
    // enforce the map immediately
    __am_shm_mmap(pp->va, pp->pa, pp->prot);
    pp->is_mapped = true;
  }
  else {
    pp->is_mapped = false;
  }
}

_Context* _ucontext(_AddressSpace *as, _Area kstack, void *entry) {
  _Context *c = (_Context*)kstack.end - 1;

  __am_get_example_uc(c);
  c->uc.uc_mcontext.gregs[REG_RIP] = (uintptr_t)entry;
  c->uc.uc_mcontext.gregs[REG_RSP] = (uintptr_t)USER_SPACE.end;

  int ret = sigemptyset(&(c->uc.uc_sigmask)); // enable interrupt
  assert(ret == 0);
  c->as = as;

  c->ksp = (uintptr_t)kstack.end;

  return c;
}

int __am_in_userspace(void *addr) {
  return vme_enable && thiscpu->cur_as != NULL && IN_RANGE(addr, USER_SPACE);
}
