#include <riscv32.h>
#include <nemu.h>
#include <klib.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN = {};
static PTE kptabs[(PMEM_SIZE + MMIO_SIZE) / PGSIZE] PG_ALIGN = {};
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static _Area segments[] = {      // Kernel memory mappings
  RANGE(0x80000000u, 0x80000000u + PMEM_SIZE),
  RANGE(MMIO_BASE, MMIO_BASE + MMIO_SIZE),
};

static inline void set_satp(void *pdir) {
  asm volatile("csrw satp, %0" : : "r"(0x80000000 | ((uintptr_t)pdir >> 12)));
}

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  // make all PDEs invalid
  int i;
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < LENGTH(segments); i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = ((uintptr_t)ptab >> PGSHFT << 10) | PTE_V;

      // fill PTE
      PTE pte = (PGADDR(pdir_idx, 0, 0) >> PGSHFT << 10) | PTE_V | PTE_R | PTE_W | PTE_X;
      PTE pte_end = (PGADDR(pdir_idx + 1, 0, 0) >> PGSHFT << 10) | PTE_V | PTE_R | PTE_W | PTE_X;
      for (; pte < pte_end; pte += (1 << 10)) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_satp(kpdirs);
  vme_enable = 1;

  return 0;
}

int _protect(_AddressSpace *as) {
  PDE *updir = (PDE*)(pgalloc_usr(1));
  as->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

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
  PDE *pt = (PDE*)as->ptr;
  PDE *pde = &pt[PDX(va)];
  if (!(*pde & PTE_V)) {
    *pde = PTE_V | ((uint32_t)pgalloc_usr(1) >> PGSHFT << 10);
  }
  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  if (!(*pte & PTE_V)) {
    *pte = PTE_V | PTE_R | PTE_W | PTE_X | ((uint32_t)pa >> PGSHFT << 10);
  }

  return 0;
}

_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack, void *entry, void *args) {
  _Context *c = (_Context*)ustack.end - 1;

  c->as = as;
  c->epc = (uintptr_t)entry;
  c->status = 0x000c0120;
  return c;
}
