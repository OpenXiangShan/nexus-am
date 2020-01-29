#include "x86_64-qemu.h"

const struct mmu_config {
  int pglevels, pgsize, pgmask;
  struct ptinfo {
    const char *name;
    uintptr_t mask;
    int shift, bits;
  } pgtables[];
} mmu = {
  .pgsize = 4096,
  .pgmask = 4095,
#if __x86_64__
  .pglevels = 4,
  .pgtables = {
    { "CR3",  0x000000000000,  0, 1 },
    { "PML4", 0xff8000000000, 39, 9 },
    { "PDPT", 0x007fc0000000, 30, 9 },
    { "PD",   0x00003fe00000, 21, 9 },
    { "PT",   0x0000001ff000, 12, 9 },
    { "PAGE", 0x000000000fff,  0, 12 },
  },
#else
  .pglevels = 2,
  .pgtables = {
    { "CR3",      0x00000000,  0, 1 },
    { "PD",       0xffc00000, 22, 10 },
    { "PT",       0x003ff000, 12, 10 },
    { "PAGE",     0x00000fff,  0, 12 },
  },
#endif
};

const struct vm_area {
  _Area area;
  int user;
} vm_areas[] = {
  { RANGE(0x800000000000, 0x808000000000), 1 },
  { RANGE(0x000000000000, 0x008000000000), 0 },
};
#define uvm_area (vm_areas[0].area)

uint64_t *PML4 = (void *)0x1000;
uint64_t *PDPT = (void *)0x2000;
static void *(*pgalloc)(size_t size);
static void (*pgfree)(void *);

static void *zalloc(size_t size) {
  uintptr_t *base = pgalloc(size);
  panic_on(!base, "cannot allocate page");
  for (int i = 0; i < size / sizeof(uintptr_t); i++) {
    base[i] = 0;
  }
  return base;
}

static uintptr_t *ptwalk(_AddressSpace *as, uintptr_t addr) {
  uintptr_t cur = (uintptr_t)&as->ptr;

  for (int i = 0; i <= mmu.pglevels; i++) {
    const struct ptinfo *ptinfo = &mmu.pgtables[i];
    uintptr_t  mask = ptinfo->mask;
    uintptr_t shift = ptinfo->shift;

    int index = ((uintptr_t)addr & mask) >> shift;
    uintptr_t *pt = (uintptr_t *)cur, next_page;
    bug_on(index >= (1 << ptinfo->bits));

    printf("level %d[%d]: %p\n", i, index, pt[index]);
    if (i == mmu.pglevels) return &pt[index];

    if (!(pt[index] & PTE_P)) {
      next_page = (uintptr_t)zalloc(mmu.pgsize);
      printf("  = allocate %p\n", next_page);
      pt[index] = next_page | PTE_P | PTE_W | PTE_U;
    } else {
      next_page = pt[index] & ~mmu.pgmask;
    }
    cur = next_page;
  }
  bug();
}

int _vme_init(void *(*f1)(size_t size), void (*f2)(void *)) {
  pgalloc = f1;
  pgfree  = f2;

  _AddressSpace as;
  _protect(&as);

  _halt(0);
  return 0;
}

int _protect(_AddressSpace *as) {
  as->pgsize = PGSIZE;
  as->area = uvm_area;
  as->ptr = NULL;
  _map(as, (void *)0x800000000000, zalloc(mmu.pgsize), _PROT_READ);
  _map(as, (void *)0x807801247000, zalloc(mmu.pgsize) + 4, _PROT_READ | _PROT_WRITE);
  return 0;
}
void _unprotect(_AddressSpace *as) {
  pgfree(as->ptr);
}

int _map(_AddressSpace *as, void *va, void *pa, int prot) {
  panic_on(!IN_RANGE(va, uvm_area), "mapping an invalid address");
  panic_on((prot & _PROT_NONE) && (prot != _PROT_NONE), "invalid protection flags");
  panic_on((uintptr_t)va != ROUNDDOWN(va, mmu.pgsize) ||
           (uintptr_t)pa != ROUNDDOWN(pa, mmu.pgsize), "non-page-boundary address");

  uintptr_t *ptentry = ptwalk(as, (uintptr_t)va);
  if (prot & _PROT_NONE) {
    panic_on(!(*ptentry & PTE_P), "unmapping an non-mapped page");
    *ptentry = 0;
  } else {
    panic_on(*ptentry & PTE_P, "remapping an existing page");
    uintptr_t pte = (uintptr_t)pa | PTE_P | PTE_U | ((prot & _PROT_WRITE) ? PTE_W : 0);
    *ptentry = pte;
  }
  return 0;
}

_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack,
                                 void *entry, void *args) { return NULL; }
