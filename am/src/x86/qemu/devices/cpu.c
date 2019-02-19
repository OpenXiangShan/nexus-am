#include "../../x86-qemu.h"


void bootcpu_init() {
  #define MAGIC 0x5f504d5f
  for (char *st = (char *)0xf0000; st != (char *)0xffffff; st ++) {
    if (*(volatile uint32_t *)st == MAGIC) {
      volatile MPConf *conf = ((volatile MPDesc *)st)->conf;
      lapic = conf->lapicaddr;
      for (volatile char *ptr = (char *)(conf + 1);
                 ptr < (char *)conf + conf->length; ptr += 8) {
        if (*ptr == '\0') {
          ptr += 12;
          if (++ncpu > MAX_CPU) {
            panic("cannot support > MAX_CPU processors");
          }
        }
      }
      return;
    }
  }
  panic("seems not an x86-qemu virtual machine");
}

void percpu_initgdt() {
  SegDesc *gdt = CPU->gdt;
  TSS *tss = &CPU->tss;
  gdt[SEG_KCODE] = SEG  (STA_X | STA_R,   0,     0xffffffff, DPL_KERN);
  gdt[SEG_KDATA] = SEG  (STA_W,           0,     0xffffffff, DPL_KERN);
  gdt[SEG_UCODE] = SEG  (STA_X | STA_R,   0,     0xffffffff, DPL_USER);
  gdt[SEG_UDATA] = SEG  (STA_W,           0,     0xffffffff, DPL_USER);
  gdt[SEG_TSS]   = SEG16(STS_T32A,      tss, sizeof(*tss)-1, DPL_KERN);
  set_gdt(gdt, sizeof(SegDesc) * NR_SEG);
  set_tr(KSEL(SEG_TSS));
}

void thiscpu_setstk0(uintptr_t ss0, uintptr_t esp0) {
  CPU->tss.ss0 = ss0;
  CPU->tss.esp0 = esp0;
}

void thiscpu_halt() {
  cli();
  while (1) hlt();
}

void othercpu_halt() {
  bootrec->is_ap = 1;
  bootrec->entry = thiscpu_halt;
  for (int cpu = 0; cpu < ncpu; cpu++) {
    if (cpu != _cpu()) {
      lapic_bootap(cpu, 0x7c00);
    }
  }
}
