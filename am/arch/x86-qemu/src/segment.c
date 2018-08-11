#include <am-x86.h>

struct TSS tss[MAX_CPU];
SegDesc gdts[MAX_CPU][NR_SEG];

void cpu_initgdt() {
  int cpu = _cpu();
  SegDesc *gdt = gdts[cpu];
  gdt[SEG_KCODE] = SEG(STA_X | STA_R,      0,       0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG(STA_W,              0,       0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG(STA_X | STA_R,      0,       0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG(STA_W,              0,       0xffffffff, DPL_USER);
  gdt[SEG_TSS]   = SEG16(STS_T32A, &tss[cpu], sizeof(tss[0])-1, DPL_KERN);
  set_gdt(gdt, sizeof(SegDesc) * NR_SEG);
  set_tr(KSEL(SEG_TSS));
}

void cpu_setustk(uintptr_t ss0, uintptr_t esp0) {
  int cpu = _cpu();
  tss[cpu].ss0 = ss0;
  tss[cpu].esp0 = esp0;
}
