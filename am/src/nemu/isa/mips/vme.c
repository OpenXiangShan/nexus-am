#include <nemu.h>
#include <klib.h>

#define USER_SPACE RANGE(0x40000000, 0x80000000)
#define PG_ALIGN __attribute((aligned(PGSIZE)))

static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static inline void *new_page() {
  void *p = pgalloc_usr(PGSIZE);
  memset(p, 0, PGSIZE);
  return p;
}

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;
  vme_enable = 1;

  return 0;
}

void _protect(_AddressSpace *as) {
  as->ptr = new_page();
  as->pgsize = PGSIZE;
  as->area = USER_SPACE;
}

void _unprotect(_AddressSpace *as) {
}

static PTE *cur_pdir = NULL;
void __am_get_cur_as(_Context *c) {
  c->pdir = cur_pdir;
}

void __am_tlb_clear();
void __am_switch(_Context *c) {
  if (vme_enable && c->pdir != NULL) {
    if (cur_pdir != c->pdir) {
      __am_tlb_clear();
      cur_pdir = c->pdir;
    }
  }
}

void _map(_AddressSpace *as, void *va, void *pa, int prot) {
  assert((uintptr_t)va % PGSIZE == 0);
  assert((uintptr_t)pa % PGSIZE == 0);
  PTE *pdir = (PTE*)as->ptr;
  PTE *pde = &pdir[PDX(va)];
  if (!(*pde & PTE_V)) {
    *pde = PTE_V | (uintptr_t)new_page();
  }
  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  if (!(*pte & PTE_V)) {
    *pte = PTE_V | PTE_D | (((uint32_t)pa >> PGSHFT) << 6);

//    printf("va = %x -> pa = %x, pte = %x\n", va, pa, pte);
    asm volatile ("mtc0 %0, $10": :"r"(va)); // hi
    asm volatile ("tlbp");

    int idx;
    asm volatile ("mfc0 %0, $0": "=r"(idx)); // index
    if (!(idx & 0x80000000u)) {
      // found
      asm volatile ("mtc0 %0, $2": : "r"(0)); // lo0
      asm volatile ("mtc0 %0, $3": : "r"(0)); // lo1
      asm volatile ("tlbwi");
    }
  }
}

_Context *_ucontext(_AddressSpace *as, _Area kstack, void *entry) {
  _Context *c = (_Context*)kstack.end - 1;
  c->pdir = as->ptr;
  c->epc = (uintptr_t)entry;
  c->status = 0x1;
  c->gpr[29] = 1; // sp slot, used as usp, non-zero
  return c;
}

void __am_tlb_refill() {
  uint32_t hi; //, lo0, lo1;
  asm volatile ("mfc0 %0, $10": "=r"(hi));

  uint32_t va = hi & ~0x1fff;
  assert(cur_pdir != NULL);
  PTE *pde = &cur_pdir[PDX(va)];
  assert(*pde & PTE_V);

  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  //assert(*pte & PTE_V);
  asm volatile ("mtc0 %0, $2": : "r"(*pte)); // lo0
  asm volatile ("mtc0 %0, $3": : "r"(*(pte + 1))); // lo1

//  printf("pte = 0x%x, lo0 = 0x%x, lo1 = 0x%x\n", pte, *pte, *(pte + 1));

  asm volatile ("tlbp");
  int idx;
  asm volatile ("mfc0 %0, $0": "=r"(idx)); // index
  if (idx & 0x80000000u) {
    // not found
    asm volatile ("tlbwr");
  }
  else {
    asm volatile ("tlbwi");
  }
}

void __am_tlb_clear() {
  asm volatile ("mtc0 %0, $10": :"r"(0)); // hi
  asm volatile ("mtc0 %0, $2": : "r"(0)); // lo0
  asm volatile ("mtc0 %0, $3": : "r"(0)); // lo1

  int i;
  for (i = 0; i < 128; i ++) {
    asm volatile ("mtc0 %0, $0": :"r"(i)); // index
    asm volatile ("tlbwi");
  }
}
