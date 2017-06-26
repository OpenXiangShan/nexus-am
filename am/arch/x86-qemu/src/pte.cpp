#include <am.h>
#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

TSS tss[MAX_CPU];
SegDesc gdts[MAX_CPU][NR_SEG];
PDE kpdirs[MAX_CPU][NR_PDE] PG_ALIGN;
PDE kptabs[MAX_CPU][NR_PDE * NR_PTE] PG_ALIGN;
void* (*palloc_f)();
void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
    {.start = (void*)0,          .end = (void*)0x10000000},  //   Low memory: kernel data
    {.start = (void*)0xf0000000, .end = (void*)(0)}, //   High memory: APIC and VGA
};

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;
  SegDesc *gdt = gdts[_cpu()];

  gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG(STA_W,         0,       0xffffffff, DPL_USER);
  gdt[SEG_TSS] = SEG16(STS_T32A,      &tss[_cpu()], sizeof(TSS)-1, DPL_KERN);
  set_gdt(gdt, sizeof(SegDesc) * NR_SEG);
  set_tr(KSEL(SEG_TSS));

  PDE *kpdir = kpdirs[_cpu()];
  PDE *alloc = kptabs[_cpu()];
  for (auto &seg: segments) {
    PTE *ptab = nullptr;
    for (uint32_t pa = reinterpret_cast<uint32_t>(seg.start); pa != reinterpret_cast<uint32_t>(seg.end); pa += PGSIZE) {
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
}

void _protect(_Protect *p) {
  PDE *kpdir = kpdirs[_cpu()];
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < 1024; i ++)
    updir[i] = kpdir[i];

  // exact copy of kernel page table
  // no user memory is mapped

  p->area.start = reinterpret_cast<void*>(0x40000000);
  p->area.end = reinterpret_cast<void*>(0xc0000000);
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
    *pde = PTE_P | PTE_W | PTE_U | reinterpret_cast<uint32_t>(palloc_f());
  }
  PTE *pte = &((PTE*)PTE_ADDR(*pde))[PTX(va)];
  if (!(*pte & PTE_P)) {
    *pte = PTE_P | PTE_W | PTE_U | reinterpret_cast<uint32_t>(pa);
  }
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  _RegSet *regs = (_RegSet*)kstack.start;
  regs->cs = USEL(SEG_UCODE);
  regs->ds = regs->es = regs->ss = USEL(SEG_UDATA);
  regs->esp3 = reinterpret_cast<uint32_t>(ustack.end);
  regs->ss0 = KSEL(SEG_KDATA);
  regs->esp0 = reinterpret_cast<uint32_t>(kstack.end);
  regs->eip = (uint32_t)entry;
  regs->eflags = FL_IF;

  uint32_t esp = regs->esp3;
  regs->esp3 = esp;
  // TODO: implement umake

  return regs;
}
