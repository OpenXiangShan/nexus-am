#include "x86.h"
#include "device.h"

SegDesc gdt[NR_SEGMENTS];       // the new GDT
TSS tss;

void
init_seg() { 
	// setup kernel segements
	gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0,       0xffffffff, DPL_KERN);
	gdt[SEG_KDATA] = SEG(STA_W,         0,       0xffffffff, DPL_KERN);
	gdt[SEG_UCODE] = SEG(STA_X | STA_R, USER_CS_BASE,       0xffffffff, DPL_USER);
	gdt[SEG_UDATA] = SEG(STA_W,         USER_SS_BASE,       0xffffffff, DPL_USER);
	gdt[SEG_TSS] = SEG16(STS_T32A,      &tss, sizeof(TSS)-1, DPL_KERN);
    	gdt[SEG_TSS].s = 0;
	set_gdt(gdt, sizeof(gdt));

	tss.esp0 = STACK;
	tss.ss0 = KSEL(SEG_KDATA);
	ltr(KSEL(SEG_TSS));
	/*设置正确的段寄存器*/
	asm volatile("movw %%ax,%%es":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ds":: "a" (KSEL(SEG_KDATA)));
	asm volatile("movw %%ax,%%ss":: "a" (KSEL(SEG_KDATA)));

	lldt(0);
}
