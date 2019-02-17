#include "../x86-qemu.h"

struct vm_area {
  _Area area;
  int physical;
};

static const struct vm_area areas[] = {
  { RANGE(0x00000000, 0x20000000), 1 }, // kernel code/data
  { RANGE(0x40000000, 0x80000000), 0 }, // protected address space range
  { RANGE(0xf0000000, 0x00000000), 1 }, // system mmap area
};
#define uvm_area (areas[1].area)

static void *pgalloc();
static void pgfree(void *ptr);

static PDE *kpt;
static void *(*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void *);

int vme_init(void *(*pgalloc_f)(size_t), void (*pgfree_f)(void *)) {
  if (_cpu() != 0) panic("init VME in non-bootstrap CPU");

  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kpt = pgalloc();
  for (int i = 0; i < NELEM(areas); i++) {
    const struct vm_area *seg = &areas[i];
    if (!seg->physical) continue;
    for (uint32_t pa =  (uint32_t)seg->area.start;
                  pa != (uint32_t)seg->area.end;
                  pa += PGSIZE) {
      PTE *ptab;
      if (!(kpt[PDX(pa)] & PTE_P)) {
        ptab = pgalloc();
        kpt[PDX(pa)] = PTE_P | PTE_W | (uint32_t)ptab;
      } else {
        ptab = (PTE*)PTE_ADDR(kpt[PDX(pa)]);
      }
      ptab[PTX(pa)] = PTE_P | PTE_W | pa;
    }
  }
  percpu_initpg(); // set CR3 and CR0 if kpt is not NULL
  return 0;
}

void percpu_initpg() { // called by all cpus
  if (kpt) {
    set_cr3(kpt);
    set_cr0(get_cr0() | CR0_PG);
  }
}

int protect(_AddressSpace *p) {
  PDE *upt = pgalloc();
  for (int i = 0; i < PGSIZE / sizeof(PDE *); i++) {
    upt[i] = kpt[i];
  }
  *p = (_AddressSpace) {
    .pgsize = PGSIZE,
    .area = uvm_area,
    .ptr = upt,
  };
  return 0;
}

void unprotect(_AddressSpace *p) {
  PDE *upt = p->ptr;
  for (uint32_t va =  (uint32_t)uvm_area.start;
                va != (uint32_t)uvm_area.end;
                va += (1 << PDXSHFT)) {
    PDE pde = upt[PDX(va)];
    if (pde & PTE_P) {
      pgfree((void *)PTE_ADDR(pde));
    }
  }
  pgfree(upt);
}

int map(_AddressSpace *p, void *va, void *pa, int prot) {
  // panic because the below cases are likely bugs
  if ((prot & _PROT_NONE) && (prot != _PROT_NONE))
    panic("invalid protection flags");
  if ((uintptr_t)va != ROUNDDOWN(va, PGSIZE) ||
      (uintptr_t)pa != ROUNDDOWN(pa, PGSIZE)) {
    panic("unaligned memory address");
  }
  if (!in_range(va, uvm_area)) {
    return 1; // mapping an out-of-range address
  }
  PDE *upt = (PDE*)p->ptr;
  PDE *pde = &upt[PDX(va)];

  if (!(*pde & PTE_P)) {
    *pde = PTE_P | PTE_W | PTE_U | (uint32_t)(pgalloc());
  }

  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  if (prot & _PROT_NONE) {
    *pte = 0; // unmap @va
  } else {
    *pte = PTE_P | ((prot & _PROT_WRITE) ? PTE_W : 0) | PTE_U
                 | (uint32_t)(pa);  // map @va -> @pa
  }
  return 0;
}

_Context *ucontext(_AddressSpace *p, _Area ustack, _Area kstack,
                                void *entry, void *args) {
  _Context *ctx = (_Context*)kstack.start;
  *ctx = (_Context) {
    .cs  = USEL(SEG_UCODE), .eip  = (uint32_t)entry, .eflags = FL_IF,
    .ds  = USEL(SEG_UDATA), .es   = USEL(SEG_UDATA),
    .ss  = USEL(SEG_UDATA), .esp3 = (uint32_t)ustack.end,
    .ss0 = KSEL(SEG_KDATA), .esp0 = (uint32_t)kstack.end,
    .eax = (uint32_t)args, // just use eax to pass @args
    .prot = p,
  };
  return ctx;
}

void *__cb_alloc(size_t size);
void __cb_free(void *ptr);

void *_cb_alloc(size_t size) { return pgalloc_usr(PGSIZE); }
void _cb_free(void *ptr) { pgfree_usr(ptr); }

static void *pgalloc() {
  void *ret = __cb_alloc(PGSIZE);
  if (!ret) panic("page allocation fail"); // for ease of debugging
  for (int i = 0; i < PGSIZE / sizeof(uint32_t); i++) {
    ((uint32_t *)ret)[i] = 0;
  }
  return ret;
}

static void pgfree(void *ptr) {
  __cb_free(ptr);
}
