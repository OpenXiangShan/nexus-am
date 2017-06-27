#include <x86-nemu.h>
#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

PDE kpdirs[NR_PDE] PG_ALIGN;
PDE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
void* (*palloc_f)();
void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
    {.start = (void*)0,          .end = (void*)0x8000000},  //   Low memory: kernel data
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  PDE *kpdir = kpdirs;
  PDE *alloc = kptabs;

  int i;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    PTE *ptab = NULL;
    for (u32 pa = (u32)segments[i].start; pa != (u32)segments[i].end; pa += PGSIZE) {
      if (!(kpdir[PDX(pa)] & PTE_P)) {
          ptab = alloc;
          alloc += NR_PDE;
          kpdir[PDX(pa)] = PTE_P | PTE_W | (u32)ptab;
      }
      ptab[PTX(pa)] = PTE_P | PTE_W | pa;
    }
  }
  
  set_cr3(kpdir);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *kpdir = kpdirs;
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < 1024; i ++)
    updir[i] = kpdir[i];

  // exact copy of kernel page table
  // no user memory is mapped

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
  // return [ULOW, UHIGH)
}

void _release(_Protect *p) {
  // free all spaces
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  PDE *pt = (PDE*)p->ptr;
  PDE *pde = &pt[PDX(va)];
  if (!(*pde & PTE_P)) {
    *pde = PTE_P | PTE_W | PTE_U | (u32)palloc_f();
  }
  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  if (!(*pte & PTE_P)) {
    *pte = PTE_P | PTE_W | PTE_U | (u32)pa;
  }
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Area ustack, _Area kstack, void *entry, int argc, char **argv) {
  return NULL;
}
