#include <am-x86.h>

static _Area prot_vm_range = RANGE(0x40000000, 0x80000000);

static _Area segments[] = {
  RANGE(0x00000000, 0x10000000),  // kernel code/data
  RANGE(0xf0000000, 0x00000000),  // system mmap area
};

static void *(*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void *);
static void *pgalloc();
static void pgfree(void *ptr);
static PDE *kpt;

int pte_init(void * (*pgalloc_f)(size_t), void (*pgfree_f)(void *)) {
  if (_cpu() != 0) {
    panic("init PTE in non-bootstrap CPU");
  }

  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kpt = pgalloc();
  for (int i = 0; i < sizeof(segments) / sizeof(segments[0]); i++) {
    _Area *seg = &segments[i];
    for (uint32_t pa = (uint32_t)seg->start;
                  pa != (uint32_t)seg->end;
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
  cpu_initpte(); // set CR3 and CR0 if kpt is not NULL
  return 0;
}

void cpu_initpte() { // called by all cpus
  if (kpt) {
    set_cr3(kpt);
    set_cr0(get_cr0() | CR0_PG);
  }
}

int protect(_Protect *p) {
  PDE *upt = pgalloc();
  for (int i = 0; i < PGSIZE / sizeof(PDE *); i++) {
    upt[i] = kpt[i];
  }
  *p = (_Protect) {
    .pgsize = PGSIZE,
    .area = prot_vm_range,
    .ptr = upt,
  };
  return 0;
}

int unprotect(_Protect *p) {
  PDE *upt = p->ptr;
  for (uint32_t va = (uint32_t)prot_vm_range.start;
                va != (uint32_t)prot_vm_range.end;
                va += (1 << PDXSHFT)) {
    PDE pde = upt[PDX(va)];
    if (pde & PTE_P) {
      pgfree((void*)PTE_ADDR(pde));
    }
  }
  pgfree(upt);
  return 0;
}

int prot_switch(_Protect *p) {
  set_cr3(p->ptr);
  return 0;
}

int map(_Protect *p, void *va, void *pa, int prot) {
  if ((prot & _PROT_NONE) && (prot != _PROT_NONE))
    panic("invalid permission");
  if ((uint32_t)va % PGSIZE != 0) panic("unaligned virtual address");
  if ((uint32_t)pa % PGSIZE != 0) panic("unaligned physical address");
  // panic because the above cases are likely bugs
  if (!in_range(va, prot_vm_range)) {
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

_Context *ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {
  _Context *ctx = (_Context*)kstack.start;
  *ctx = (_Context) {
    .cs = USEL(SEG_UCODE),  .eip = (uint32_t)entry, .eflags = FL_IF,
    .ds = USEL(SEG_UDATA),  .es  = USEL(SEG_UDATA),
    .ss  = USEL(SEG_UDATA), .esp3 = (uint32_t)ustack.end,
    .ss0 = KSEL(SEG_KDATA), .esp0 = (uint32_t)kstack.end,
    .eax = (uint32_t)args,
  };
  return ctx;
}

static void *pgalloc() {
  void *ret = pgalloc_usr(PGSIZE);
  if (!ret) panic("page allocation fail"); // for ease of debugging
  for (int i = 0; i < PGSIZE / sizeof(uint32_t); i++) {
    ((uint32_t *)ret)[i] = 0;
  }
  return ret;
}

static void pgfree(void *ptr) {
  pgfree_usr(ptr);
}
