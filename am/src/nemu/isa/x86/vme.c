#include <am.h>
#include <nemu.h>
#include <klib.h>

#define PTE_ADDR(pte)    ((uintptr_t)(pte) & ~0xfff)

static _AddressSpace kas; // Kernel address space
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static _Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE,
};

#define USER_SPACE RANGE(0x40000000, 0xc0000000)

static inline void *new_page() {
  void *p = pgalloc_usr(PGSIZE);
  memset(p, 0, PGSIZE);
  return p;
}

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = new_page();
  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      _map(&kas, va, va, 0);
    }
  }

  set_cr3(kas.ptr);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return 0;
}

void _protect(_AddressSpace *as) {
  PTE *updir = new_page();
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void _unprotect(_AddressSpace *as) {
}

void __am_get_cur_as(_Context *c) {
  c->cr3 = (vme_enable ? (void *)get_cr3() : NULL);
}

void __am_switch(_Context *c) {
  if (vme_enable && c->cr3 != NULL) { set_cr3(c->cr3); }
}

#define PTW_CONFIG ((ptw_config) { .ptw_level = 2, .vpn_width = 10 })

void _map(_AddressSpace *as, void *va, void *pa, int prot) {
  assert((uintptr_t)va % PGSIZE == 0);
  assert((uintptr_t)pa % PGSIZE == 0);
  PTE *pg_base = as->ptr;
  PTE *pte;
  int level;
  for (level = PTW_CONFIG.ptw_level - 1; ; level --) {
    pte = &pg_base[VPNi(PTW_CONFIG, (uintptr_t)va, level)];
    pg_base = (PTE *)PTE_ADDR(*pte);
    if (level == 0) break;
    if (!(*pte & PTE_P)) {
      pg_base = new_page();
      *pte = PTE_P | PTE_W | PTE_U | (uintptr_t)pg_base;
    }
  }

  if (!(*pte & PTE_P)) {
    *pte = PTE_P | PTE_W | PTE_U | (uintptr_t)pa;
  }
}

_Context* _ucontext(_AddressSpace *as, _Area kstack, void *entry) {
  _Context *c = (_Context *)kstack.end - 1;
  c->cr3 = as->ptr;
  c->cs = 0x8;
  c->eip = (uintptr_t)entry;
  c->eflags = 0x2 | FL_IF;
  c->usp = (uintptr_t)kstack.end; // non-zero but should be used to safely construct context
  return c;
}
