#ifndef __AM_X86_H__
#define __AM_X86_H__

void lapic_eoi();
void lapic_init();
void ioapic_init();
void lapic_bootap(int cpu, uint32_t address);
void ioapic_enable(int irq, int cpu);
void smp_init();

static inline void puts(const char *s) {
  for (; *s; s++) {
    _putc(*s);
  }
}

#define STRINGIFY(s) #s
#define TOSTRING(s) STRINGIFY(s)

#define panic(s) \
  do { \
    puts("AM Panic: "); puts(s); \
    puts(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n"); \
    _halt(1); \
  } while(0)

#endif
