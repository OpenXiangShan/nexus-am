#include <am-x86.h>

TSS tss[MAX_CPU];
SegDesc gdts[MAX_CPU][NR_SEG];

void percpu_initgdt() {
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

void thiscpu_setustk(uintptr_t ss0, uintptr_t esp0) {
  int cpu = _cpu();
  tss[cpu].ss0 = ss0;
  tss[cpu].esp0 = esp0;
}

void thiscpu_halt() {
  cli();
  while (1) hlt();
}

void othercpu_halt() {
  boot_rec->is_ap = 1;
  boot_rec->entry = thiscpu_halt;
  for (int cpu = 0; cpu < ncpu; cpu++) {
    if (cpu != _cpu()) {
      lapic_bootap(cpu, 0x7c00);
    }
  }
}

#define MP_PROC    0x00
#define MP_MAGIC   0x5f504d5f // _MP_

void bootcpu_init() {
  for (char *st = (char *)0xf0000; st != (char *)0xffffff; st ++) {
    if (*(volatile uint32_t *)st == MP_MAGIC) {
      volatile MPConf *conf = ((volatile MPDesc *)st)->conf;
      lapic = conf->lapicaddr;
      for (volatile char *ptr = (char *)(conf + 1);
                 ptr < (char *)conf + conf->length; ) {
        if (*ptr == MP_PROC) {
          ptr += 20;
          if (++ncpu > MAX_CPU) {
            panic("cannot support > MAX_CPU processors");
          }
        } else {
          ptr += 8;
        }
      }
      return;
    }
  }
  panic("seems not an x86-qemu machine");
}
