#include "platform.h"

#define USER_SPACE (_Area) { .start = (void *)0x40000000ul, .end = (void *)0xc0000000ul }

#define PGSIZE  4096

typedef struct PageMap {
  void *va;
  void *pa;
  struct PageMap *next;
  int prot;
  int is_mapped;
} PageMap;

#define list_foreach(p, head) \
  for (p = head; p != NULL; p = p->next)

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
  as->pgsize = PGSIZE;
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
      assert(USER_SPACE.start <= pp->va && pp->va < USER_SPACE.end);
      __am_shm_mmap(pp->va, pp->pa, pp->prot);
      pp->is_mapped = true;
    }
  }

  thiscpu->cur_as = as;
}

void _map(_AddressSpace *as, void *va, void *pa, int prot) {
  assert(USER_SPACE.start <= va && va < USER_SPACE.end);
  assert((uintptr_t)va % PGSIZE == 0);
  assert((uintptr_t)pa % PGSIZE == 0);
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

  pp = pgalloc(PGSIZE); // this will waste memory, any better idea?
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

void _ucontext(_Context *c, _AddressSpace *as, _Area kstack, void *entry) {
  kstack.end -= RED_NONE_SIZE;
  _Context *c_on_stack = (_Context*)kstack.end - 1;

  __am_get_example_uc(c);
  c->rip = (uintptr_t)entry;
  c->sti = 1;
  c->rflags = 0;
  c->as = as;
  //c->esp = 0;  FIXME: we leave the esp undefined
  c->uc.uc_mcontext.gregs[REG_RDI] = (uintptr_t)c_on_stack; // used in __am_irq_handle()
}

int __am_in_userspace(void *addr) {
  return vme_enable && (USER_SPACE.start <= addr) && (addr < USER_SPACE.end);
}
