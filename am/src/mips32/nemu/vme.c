#include <mips32.h>
#include <klib.h>
#include <nemu.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  return 0;
}

int _protect(_AddressSpace *p) {
  p->ptr = (PDE*)(pgalloc_usr(1));
  p->pgsize = 4096;

  return 0;
}

void _unprotect(_AddressSpace *p) {
}

static _AddressSpace *cur_as = NULL;
void __am_get_cur_as(_Context *c) {
  c->prot = cur_as;
}

void __am_tlb_clear();
void __am_switch(_Context *c) {
  if (cur_as != NULL && cur_as->ptr != c->prot->ptr) {
    __am_tlb_clear();
  }
  cur_as = c->prot;
}

int _map(_AddressSpace *p, void *va, void *pa, int mode) {
  PDE *pt = (PDE*)p->ptr;
  PDE *pde = &pt[PDX(va)];
  if (!(*pde & PTE_V)) {
    *pde = PTE_V | (uint32_t)pgalloc_usr(1);
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

  return 0;
}

_Context *_ucontext(_AddressSpace *p, _Area ustack, _Area kstack, void *entry, void *args) {
  _Context *c = (_Context*)ustack.end - 1;

  c->prot = p;
  c->epc = (uintptr_t)entry;
  c->status = 0x1;
  return c;
}

void __am_tlb_refill() {
  uint32_t hi; //, lo0, lo1;
  asm volatile ("mfc0 %0, $10": "=r"(hi));

  uint32_t va = hi & ~0x1fff;
  PDE *pt = (PDE*)cur_as->ptr;
  PDE *pde = &pt[PDX(va)];
//  if (!(*pde & PTE_V)) {
//    printf("hi = 0x%x, pt = 0x%x\n", hi, pt);
//  }
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
