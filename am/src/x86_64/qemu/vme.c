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
    { "CR3",  0x000000000000,  0, 0 },
    { "PML4", 0xff8000000000, 39, 9 },
    { "PDPT", 0x007fc0000000, 30, 9 },
    { "PD",   0x00003fe00000, 21, 9 },
    { "PT",   0x0000001ff000, 12, 9 },
    { "PAGE", 0x000000000fff,  0, 12 },
  },
#else
  .pglevels = 2,
  .pgtables = {
    { "CR3",      0x00000000,  0, 0 },
    { "PD",       0xffc00000, 22, 10 },
    { "PT",       0x003ff000, 12, 10 },
    { "PAGE",     0x00000fff,  0, 12 },
  },
#endif
};

struct vm_area {
  _Area area;
  int kernel;
};

static const struct vm_area vm_areas[] = {
#ifdef __x86_64__
  { RANGE(0x100000000000, 0x108000000000), 0 }, // 512 GiB user space
  { RANGE(0x000000000000, 0x008000000000), 1 }, // 512 GiB kernel
#else
  { RANGE(    0x40000000,     0x80000000), 0 }, // 1 GiB user space
  { RANGE(    0x00000000,     0x40000000), 1 }, // 1 GiB kernel
  { RANGE(    0xfd000000,     0x00000000), 1 }, // memory-mapped I/O
#endif
};
#define uvm_area (vm_areas[0].area)

static uintptr_t *kpt;
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

static int indexof(uintptr_t addr, const struct ptinfo *info) {
  return ((uintptr_t)addr & info->mask) >> info->shift;
}

static uintptr_t baseof(uintptr_t addr) {
  return addr & ~mmu.pgmask;
}

static uintptr_t *ptwalk(_AddressSpace *as, uintptr_t addr, int flags) {
  uintptr_t cur = (uintptr_t)&as->ptr;

  for (int i = 0; i <= mmu.pglevels; i++) {
    const struct ptinfo *ptinfo = &mmu.pgtables[i];
    uintptr_t *pt = (uintptr_t *)cur, next_page;
    int index = indexof(addr, ptinfo);
//    printf("level %d[%d]: %p\n", i, index, pt[index]);
    if (i == mmu.pglevels) return &pt[index];

    if (!(pt[index] & PTE_P)) {
      next_page = (uintptr_t)zalloc(mmu.pgsize);
//      printf("  = allocate %p\n", next_page);
      pt[index] = next_page | PTE_P | flags;
    } else {
      next_page = baseof(pt[index]);
    }
    cur = next_page;
  }
  bug();
}

static void teardown(int level, uintptr_t *pt) {
  if (level > mmu.pglevels) return;
  for (int index = 0; index < (1 << mmu.pgtables[level].bits); index++) {
    if (pt[index] & PTE_P) {
      teardown(level + 1, (void *)(pt[index] & ~mmu.pgmask));
    }
  }
  if (level >= 1) {
    pgfree(pt);
  }
}

int _vme_init(void *(*f1)(size_t size), void (*f2)(void *)) {
  panic_on(_cpu() != 0, "init VME in non-bootstrap CPU");
  pgalloc = f1;
  pgfree  = f2;

#if __x86_64__
  kpt = (void *)0x1000; // PML4
#else
  _AddressSpace as;
  as.ptr = NULL;
  for (int i = 0; i < LENGTH(vm_areas); i++) {
    const struct vm_area *vma = &vm_areas[i];
    if (vma->kernel) {
      for (uintptr_t cur = (uintptr_t)vma->area.start;
           cur != (uintptr_t)vma->area.end;
           cur += mmu.pgsize) {
        *ptwalk(&as, cur, PTE_W) = cur | PTE_P | PTE_W;
      }
    }
  }
  kpt = (void *)baseof((uintptr_t)as.ptr);
#endif

  set_cr3(kpt);
  set_cr0(get_cr0() | CR0_PG);
  return 0;
}

int _protect(_AddressSpace *as) {
  uintptr_t *upt = zalloc(mmu.pgsize);

  for (int i = 0; i < LENGTH(vm_areas); i++) {
    const struct vm_area *vma = &vm_areas[i];
    if (vma->kernel) {
      const struct ptinfo *info = &mmu.pgtables[1]; // level-1 page table
      for (uintptr_t cur = (uintptr_t)vma->area.start;
           cur != (uintptr_t)vma->area.end;
           cur += (1L << info->shift)) {
        int index = indexof(cur, info);
        upt[index] = kpt[index];
      }
    }
  }

  *as = (_AddressSpace) {
    .pgsize = mmu.pgsize,
    .area   = uvm_area,
    .ptr    = (void *)((uintptr_t)upt | PTE_P),
  };

  set_cr3(as->ptr);
 
  return 0;
}

void _unprotect(_AddressSpace *as) {
  teardown(0, (void *)&as->ptr);
}

int _map(_AddressSpace *as, void *va, void *pa, int prot) {
  panic_on(!IN_RANGE(va, uvm_area), "mapping an invalid address");
  panic_on((prot & _PROT_NONE) && (prot != _PROT_NONE), "invalid protection flags");
  panic_on((uintptr_t)va != ROUNDDOWN(va, mmu.pgsize) ||
           (uintptr_t)pa != ROUNDDOWN(pa, mmu.pgsize), "non-page-boundary address");

  printf("map %p -> %p, %d\n", va, pa, prot);
  uintptr_t *ptentry = ptwalk(as, (uintptr_t)va, PTE_W | PTE_U);
  printf("ptentry: %p\n", ptentry);
  if (prot & _PROT_NONE) {
    panic_on(!(*ptentry & PTE_P), "unmapping an non-mapped page");
    *ptentry = 0;
  } else {
    panic_on(*ptentry & PTE_P, "remapping an existing page");
    uintptr_t pte = (uintptr_t)pa | PTE_P | PTE_U | ((prot & _PROT_WRITE) ? PTE_W : 0);
    *ptentry = pte;
  }
  ptwalk(as, (uintptr_t)va, PTE_W | PTE_U);
  return 0;
}

_Context *_ucontext(_AddressSpace *as, _Area ustack, _Area kstack, void *entry, void *args) {
  _Area stk_aligned = {
    (void *)ROUNDUP(kstack.start, 16),
    (void *)ROUNDDOWN(kstack.end, 16),
  };
  uintptr_t stk_top = (uintptr_t)stk_aligned.end;

  _Context *ctx = (_Context *)stk_aligned.start;
  *ctx = (_Context) { 0 };

#if __x86_64__
  ctx->cs = USEL(SEG_UCODE);
  ctx->rip = (uintptr_t)entry;
  ctx->rflags = FL_IF;
  ctx->ss = USEL(SEG_UDATA);
  ctx->rsp = ROUNDDOWN(ustack.end, 16);
  ctx->rsp0 = stk_top;
#else
  ctx->cs = USEL(SEG_UCODE);
  ctx->eip = (uint32_t)entry;
  ctx->eflags = FL_IF;
  ctx->ds = ctx->ss3 = USEL(SEG_UDATA);
  ctx->esp0 = stk_top;
  ctx->esp = ROUNDDOWN(ustack.end, 16);
#endif
  ctx->GPRx = (uintptr_t)args;
  ctx->uvm = as->ptr;
  printf("uvm (cr3) = %p\n", ctx->uvm);
  return ctx;
}
