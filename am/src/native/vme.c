#include "platform.h"

#define USER_SPACE (_Area) { .start = (void *)0x40000000ul, .end = (void *)0xc0000000ul }

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

static _AddressSpace empty_as = { .ptr = NULL };
static _AddressSpace **cur_as = NULL; // per-cpu
static int vme_enable = 0;

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  // we do not need to ask MM to get a page from OS,
  // since we can call malloc() in native
  cur_as = __am_private_alloc(sizeof(*cur_as));
  *cur_as = &empty_as;

  vme_enable = 1;
  return 0;
}

int _protect(_AddressSpace *as) {
  as->ptr = NULL;
  as->pgsize = PGSIZE;
  as->area = USER_SPACE;
  return 0;
}

void _unprotect(_AddressSpace *as) {
}

void __am_get_cur_as(_Context *c) {
  c->as = (vme_enable ? *cur_as : &empty_as);
}

void __am_get_empty_as(_Context *c) {
  c->as = &empty_as;
}

void __am_switch(_Context *c) {
  if (!vme_enable) return;

  _AddressSpace *as = c->as;
  assert(as != NULL);
  if (as == *cur_as) return;

  PageMap *pp;
  // munmap all mappings
  list_foreach(pp, (*cur_as)->ptr) {
    if (pp->is_mapped) {
      __am_shm_munmap((void *)(pp->vpn << PGSHIFT));
      pp->is_mapped = false;
    }
  }

  // mmap all mappings
  list_foreach(pp, as->ptr) {
    __am_shm_mmap((void *)(pp->vpn << PGSHIFT), (void *)(pp->ppn << PGSHIFT), 0);
    pp->is_mapped = true;
  }

  *cur_as = as;
}

int _map(_AddressSpace *as, void *va, void *pa, int prot) {
  uintptr_t vpn = (uintptr_t)va >> PGSHIFT;
  PageMap *pp;
  list_foreach(pp, as->ptr) {
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
  pp->next = as->ptr;
  as->ptr = pp;

  if (as == *cur_as) {
    // enforce the map immediately
    __am_shm_mmap((void *)(pp->vpn << PGSHIFT), (void *)(pp->ppn << PGSHIFT), 0);
    pp->is_mapped = true;
  }
  else {
    pp->is_mapped = false;
  }

  return 0;
}

_Context *_ucontext(_AddressSpace *as, _Area kstack, void *entry) {
  kstack.end -= 1 * sizeof(uintptr_t);  // 1 = retaddr
  uintptr_t ret = (uintptr_t)kstack.end;
  *(uintptr_t *)ret = 0;

  kstack.end -= RED_NONE_SIZE;
  _Context *c = (_Context*)kstack.end - 1;
  __am_get_example_uc(c);
  c->rip = (uintptr_t)entry;
  int ret2 = sigemptyset(&(c->uc.uc_sigmask)); // enable interrupt
  assert(ret2 == 0);
  c->rflags = 0;
  c->event = _EVENT_YIELD;
  c->as = as;
  c->uc.uc_mcontext.gregs[REG_RDI] = (uintptr_t)c; // used in __am_irq_handle()

  return c;
}
