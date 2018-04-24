#ifndef __AM_X86_H__
#define __AM_X86_H__

void lapic_eoi();
void lapic_init();
void ioapic_init();
void lapic_bootap(int cpu, uint32_t address);
void ioapic_enable(int irq, int cpu);
void smp_init();

#endif