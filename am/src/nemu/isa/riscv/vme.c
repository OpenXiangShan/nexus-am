#include <nemu.h>
#include <klib.h>

static _AddressSpace kas; // Kernel address space
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static const _Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE,
#if __riscv_xlen == 64
  RANGE(0xa2000000, 0xa2000000 + 0x10000), // clint
#endif
};

#if __riscv_xlen == 64
#define USER_SPACE RANGE(0xc0000000, 0xf0000000)
#define SATP_MODE (8ull << 60)
#define PTW_CONFIG PTW_SV39
#else
#define USER_SPACE RANGE(0x40000000, 0x80000000)
#define SATP_MODE 0x80000000
#define PTW_CONFIG PTW_SV32
#endif

static inline void set_satp(void *pdir) {
  asm volatile("csrw satp, %0" : : "r"(SATP_MODE | PN(pdir)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;  // the mode bits will be shifted out
}

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
  // make all PTEs invalid
  memset(kas.ptr, 0, PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      _map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return 0;
}

void _protect(_AddressSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void _unprotect(_AddressSpace *as) {
}

void __am_get_cur_as(_Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(_Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

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
    if (!(*pte & PTE_V)) {
      pg_base = pgalloc_usr(PGSIZE);
      *pte = PTE_V | (PN(pg_base) << 10);
    }
  }

  if (!(*pte & PTE_V)) {
    *pte = PTE_V | PTE_R | PTE_W | PTE_X | (PN(pa) << 10);
  }
}

_Context *_ucontext(_AddressSpace *as, _Area kstack, void *entry) {
  _Context *c = (_Context*)kstack.end - 1;

  c->pdir = as->ptr;
  c->sepc = (uintptr_t)entry;
  c->sstatus = MSTATUS_SPP(MODE_S) | MSTATUS_PIE(MODE_S);
  c->gpr[2] = 1; // sp slot, used as usp, non-zero is ok
  return c;
}
