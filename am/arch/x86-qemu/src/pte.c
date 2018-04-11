#include <am.h>
#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

struct TSS tss[MAX_CPU];
SegDesc gdts[MAX_CPU][NR_SEG];
PDE kpdir[NR_PDE] PG_ALIGN;
PDE kptab[NR_PDE * NR_PTE] PG_ALIGN;
void* (*palloc_f)(size_t);
void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
    {.start = (void*)0,          .end = (void*)0x10000000},  //   Low memory: kernel data
    {.start = (void*)0xf0000000, .end = (void*)(0)}, //   High memory: APIC and VGA
};

int _pte_init(void* (*palloc)(size_t), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;
  SegDesc *gdt = gdts[_cpu()];

  gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
  gdt[SEG_TSS] = SEG16(STS_T32A,      &tss[_cpu()], sizeof(struct TSS)-1, DPL_KERN);
  set_gdt(gdt, sizeof(SegDesc) * NR_SEG);
  set_tr(KSEL(SEG_TSS));

  PDE *alloc = kptab;
  for (int i = 0; i < sizeof(segments) / sizeof(segments[0]); i++) {
    _Area *seg = &segments[i];
    PTE *ptab = NULL;
    for (uint32_t pa = (uint32_t)seg->start; pa != (uint32_t)seg->end; pa += PGSIZE) {
      if (!(kpdir[PDX(pa)] & PTE_P)) {
          ptab = alloc;
          alloc += NR_PDE;
          kpdir[PDX(pa)] = PTE_P | PTE_W | (uint32_t)ptab;
      }
      ptab[PTX(pa)] = PTE_P | PTE_W | pa;
    }
  }
  
  set_cr3(kpdir);
  set_cr0(get_cr0() | CR0_PG);
  return 0;
}

int _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f(PGSIZE));
  p->pgsize = PGSIZE;
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < 1024; i ++)
    updir[i] = kpdir[i];

  // exact copy of kernel page table
  // no user memory is mapped

  p->area.start = (void*)0x40000000;
  p->area.end = (void*)0xc0000000;
  return 0;
}

void _unprotect(_Protect *p) {
  // DFS pages and call release
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

int _map(_Protect *p, void *va, void *pa, int prot) {
  PDE *pt = (PDE*)p->ptr;
  PDE *pde = &pt[PDX(va)];
  uint32_t wflag = 0; // TODO: this should be not accessible
  if (prot & _PROT_WRITE) wflag = PTE_W;
  if (!(*pde & PTE_P)) {
    *pde = PTE_P | wflag | PTE_U | (uint32_t)(palloc_f(PGSIZE));
  }
  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  if (!(*pte & PTE_P)) {
    *pte = PTE_P | wflag | PTE_U | (uint32_t)(pa);
  }
  return 0;
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void (*entry)(void *), void *args) {
  _RegSet *regs = (_RegSet*)kstack.start;
  regs->cs = USEL(SEG_UCODE);
  regs->ds = regs->es = regs->ss = USEL(SEG_UDATA);
  regs->esp3 = (uint32_t)ustack.end;
  regs->ss0 = KSEL(SEG_KDATA);
  regs->esp0 = (uint32_t)kstack.end;
  regs->eip = (uint32_t)entry;
  regs->eflags = FL_IF;

  uint32_t esp = regs->esp3;
  regs->esp3 = esp;
  // TODO:
  // push args to ustack
  // prepare kstack
  // make every register correct

  return regs;
}
