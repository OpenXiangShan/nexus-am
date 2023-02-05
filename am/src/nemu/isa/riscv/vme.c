#include <nemu.h>
#include <klib.h>

#define CLINT_MMIO (RTC_ADDR - 0xbff8)

_AddressSpace kas; // Kernel address space
static void* (*pgalloc_usr)(size_t) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

#define RANGE_LEN(start, len) RANGE((start), (start + len))

static const _Area segments[] = {      // Kernel memory mappings
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV64_XS)
  RANGE_LEN(0x80000000, 0x8000000), // PMEM
  RANGE_LEN(0x40600000, 0x1000),    // uart
  RANGE_LEN(CLINT_MMIO, 0x10000),   // clint/timer
  RANGE_LEN(FB_ADDR,    0x400000),  // vmem
  RANGE_LEN(SCREEN_ADDR,0x1000),    // vmem
  RANGE_LEN(0x3c000000, 0x4000000), // PLIC
  RANGE_LEN(0xc0000000, 0x100000),  // page table test allocates from this position
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  RANGE_LEN(0x2000000000, 0x800000), // PMEM
  RANGE_LEN(0x1f00050000, 0x1000),    // uart
  // RANGE_LEN(CLINT_MMIO, 0x10000),     // clint/timer
  // RANGE_LEN(0x1f0c000000, 0x4000000), // PLIC
  RANGE_LEN(0x2040000000, 0x100000),  // page table test allocates from this position
#else
  NEMU_PADDR_SPACE,
#if __riscv_xlen == 64
  RANGE_LEN(0xa2000000, 0x10000), // clint
#endif
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
#if __riscv_xlen == 64
  asm volatile("sfence.vma");
#endif
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;  // the mode bits will be shifted out
}

static inline void *new_page() {
  void *p = pgalloc_usr(PGSIZE);
  memset(p, 0, PGSIZE);
  return p;
}

/*
 * Virtual Memory initialize
 * pgalloc_f: pointer of page table memory allocater, must return page-aligned address
 * pgfree_f: pointer page table memory free function
 * return 0 if success
 */
int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = new_page();
  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    printf("va start %llx, end %llx\n", segments[i].start, segments[i].end);
    for (; va < segments[i].end; va += PGSIZE) {
      _map(&kas, va, va, PTE_R | PTE_W | PTE_X | PTE_A | PTE_D);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return 0;
}

/*
 * Virtual Memory initialize with custom segments
 * pgalloc_f: pointer of page table memory allocater, must return page-aligned address
 * pgfree_f: pointer page table memory free function
 * custom_segments: pointer of self-defined segments
 * len: length of custom_segments
 * return 0 if success
 */
int _vme_init_custom(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*), _Area custom_segments[], int len) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = new_page();
  int i;
  for (i = 0; i < len; i ++) {
    void *va = custom_segments[i].start;
    printf("va start %llx, end %llx\n", custom_segments[i].start, custom_segments[i].end);
    for (; va < custom_segments[i].end; va += PGSIZE) {
      _map(&kas, va, va, PTE_R | PTE_W | PTE_X | PTE_A | PTE_D);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return 0;
}

/*
 * copy page table
 */
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

/*
 * get current satp
 */
void __am_get_cur_as(_Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

/*
 * switch page table to the given context
 */
void __am_switch(_Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

/*
 * map va to pa with prot permission with page table root as
 * Note that RISC-V allow hardware to fault when A and D bit is not set
 */
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
      pg_base = new_page();
      *pte = PTE_V | (PN(pg_base) << 10);
    }
  }

  if (!(*pte & PTE_V)) {
    *pte = PTE_V | prot | (PN(pa) << 10);
  }
}

/*
 * A wrong map to test access fault, high bits of ppn is not zero
 * Only available for sv39 and paddr = 36! (ppnlen can not be changed)
 */
void _map_fault(_AddressSpace *as, void *va, void *pa, int prot) {
  assert((uintptr_t)va % PGSIZE == 0);
  assert((uintptr_t)pa % PGSIZE == 0);
  PTE *pg_base = as->ptr;
  PTE *pte;
  int level;
  uintptr_t max = 1;
  for (int i = 0; i < 20; i++){
    max *= 2;
  }
  uintptr_t randnum = rand() % (max - 1);

  for (level = PTW_CONFIG.ptw_level - 1; ; level --) {
    pte = &pg_base[VPNi(PTW_CONFIG, (uintptr_t)va, level)];
    pg_base = (PTE *)PTE_ADDR(*pte);
    if (level == 0) break;
    if (!(*pte & PTE_V)) {
      pg_base = new_page();
      *pte = PTE_V | (PN(pg_base) << 10);
    }
  }

  if (!(*pte & PTE_V)) {
    *pte = PTE_V | prot | (PN(pa) << 10) | (randnum << 34);
  }
}

/*
 * map va to pa with prot permission with page table root as
 * pagetable_level indicates page table level to be used
 * 0: basic 4KiB page
 * 1: 2MiB megapage
 * 2: 1GiB gigapage
 * Note that RISC-V allow hardware to fault when A and D bit is not set
 */
void _map_rv_hugepage(_AddressSpace *as, void *va, void *pa, int prot, int pagetable_level) {
  int hugepage_size;
  switch (pagetable_level) {
    case 0: hugepage_size = PGSIZE; break; // 4KiB
    case 1: hugepage_size = PGSIZE * 512; break;  // 2MiB
    case 2: hugepage_size = PGSIZE * 512 * 512; break;  // 1GiB
    default: assert(0);
  }
  assert((uintptr_t)va % hugepage_size == 0);
  // printf("pa %lx sz %lx\n", pa, hugepage_size);
  assert((uintptr_t)pa % hugepage_size == 0);
  PTE *pg_base = as->ptr;
  PTE *pte;
  int level;
  for (level = PTW_CONFIG.ptw_level - 1; ; level --) {
    pte = &pg_base[VPNi(PTW_CONFIG, (uintptr_t)va, level)];
    pg_base = (PTE *)PTE_ADDR(*pte);
    if (level == pagetable_level) break;
    if (!(*pte & PTE_V)) {
      pg_base = new_page();
      *pte = PTE_V | (PN(pg_base) << 10);
    }
  }

  int hugepage_pn_shift = pagetable_level * 9;
  if (!(*pte & PTE_V)) {
    *pte = PTE_V | prot | (PN(pa) >> hugepage_pn_shift << hugepage_pn_shift << 10);
  }
  printf("map huge page level %d, pte value %lx\n", pagetable_level, *pte);
}

_Context *_ucontext(_AddressSpace *as, _Area kstack, void *entry) {
  _Context *c = (_Context*)kstack.end - 1;

  c->pdir = as->ptr;
  c->sepc = (uintptr_t)entry;
  c->sstatus = MSTATUS_SPP(MODE_S) | MSTATUS_PIE(MODE_S);
  c->gpr[2] = 1; // sp slot, used as usp, non-zero is ok
  return c;
}
