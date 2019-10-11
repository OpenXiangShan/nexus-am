#include <riscv64.h>
#include <nemu.h>
#include <klib.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static _AddressSpace kas; // Kernel address space
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static _Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0x80000000u, .end = (void*)(0x80000000u + PMEM_SIZE)},
  {.start = (void*)MMIO_BASE,   .end = (void*)(MMIO_BASE + MMIO_SIZE)}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

static inline void set_satp(void *pdir) {
  uintptr_t mode = 4ull << 60;
  asm volatile("csrw satp, %0" : : "r"(mode | PN(pdir)));
}

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(1);
  // make all PTEs invalid
  memset(kas.ptr, 0, PGSIZE);

  int i;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      _map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return 0;
}

int _protect(_AddressSpace *as) {
  PTE *updir = (PTE *)(pgalloc_usr(1));
  as->ptr = updir;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);

  return 0;
}

void _unprotect(_AddressSpace *as) {
}

static _AddressSpace *cur_as = NULL;
void __am_get_cur_as(_Context *c) {
  c->as = cur_as;
}

void __am_switch(_Context *c) {
  if (vme_enable) {
    set_satp(c->as->ptr);
    cur_as = c->as;
  }
}

int _map(_AddressSpace *as, void *va, void *pa, int prot) {
  PTE *pte2 = &((PTE*)as->ptr)[VPN2(va)];
  if (!(*pte2 & PTE_V)) {
    *pte2 = PTE_V | (PN(pgalloc_usr(1)) << 10);
  }
  PTE *pte1 = &((PTE*)PTE_ADDR(*pte2))[VPN1(va)];
  if (!(*pte1 & PTE_V)) {
    *pte1 = PTE_V | (PN(pgalloc_usr(1)) << 10);
  }
  PTE *pte0 = &((PTE*)PTE_ADDR(*pte1))[VPN0(va)];
  if (!(*pte0 & PTE_V)) {
    *pte0 = PTE_V | PTE_R | PTE_W | PTE_X | (PN(pa) << 10);
  }

  return 0;
}

_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack, void *entry, void *args) {
  _Context *c = (_Context*)ustack.end - 1;
  c->gpr[10] = c->gpr[11] = 0;

  c->as = as;
  c->mepc = (uintptr_t)entry;
  c->mstatus = 0x000c0180;
  return c;
}
