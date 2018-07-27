#include <am.h>
#include <stdlib.h>
#include <klib.h>

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

int _pte_init(void* (*palloc)(size_t), void (*pfree)(void*)) {
  return 0;
}

int _protect(_Protect *p) {
  p->ptr = NULL;
  return 0;
}

static _Protect *cur_as = NULL;

void _switch(_Protect *p) {
  if (p == cur_as) return;
  PageMap *pp;

  if (cur_as != NULL) {
    // munmap all mappings
    list_foreach(pp, cur_as->ptr) {
      if (pp->is_mapped) {
        shm_munmap((void *)(pp->vpn << 12));
        pp->is_mapped = false;
      }
    }
  }

  // mmap all mappings
  list_foreach(pp, p->ptr) {
    shm_mmap((void *)(pp->vpn << 12), (void *)(pp->ppn << 12), 0);
    pp->is_mapped = true;
  }

  cur_as = p;
}

int _map(_Protect *p, void *va, void *pa, int prot) {
  uintptr_t vpn = (uintptr_t)va >> 12;
  PageMap *pp;
  list_foreach(pp, p->ptr) {
    // can not remap
    if (pp->vpn == vpn) {
      printf("check remap: %p -> %p, but previously %p -> %p\n", va, pa, pp->vpn << 12, pp->ppn << 12);
      assert(pp->ppn == ((uintptr_t)pa >> 12));
      return 0;
    }
  }

  pp = malloc(sizeof(PageMap));
  pp->vpn = vpn;
  pp->ppn = (uintptr_t)pa >> 12;
  pp->next = p->ptr;
  p->ptr = pp;

  if (p == cur_as) {
    // enforce the map immediately
    shm_mmap((void *)(pp->vpn << 12), (void *)(pp->ppn << 12), 0);
    pp->is_mapped = true;
  }
  else {
    pp->is_mapped = false;
  }
  
  return 0;
}

void get_example_uc(_RegSet *r);

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void (*entry)(void *), void *args) {
  ustack.end -= 4 * sizeof(uintptr_t);  // 4 = retaddr + argc + argv + envp
  _RegSet *r = (_RegSet*)ustack.end - 1;

  get_example_uc(r);
  r->rip = (uintptr_t)entry;
  return r;
}
