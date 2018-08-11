#ifndef __AM_X86_H__
#define __AM_X86_H__

#include <am.h>
#include <amdev.h>
#include <amtrace.h>
#include <x86.h>

extern volatile uint32_t *lapic;
extern int ncpu;
extern volatile uint32_t trace_flags;

void lapic_eoi();
void lapic_init();
void ioapic_init();
void lapic_bootap(int cpu, uint32_t address);
void ioapic_enable(int irq, int cpu);
void cpu_initidt();
void cpu_initgdt();
void cpu_initpte();
void cpu_setustk(uintptr_t ss0, uintptr_t esp0);
void cpu_die();
void mp_halt();

#define RANGE(st, ed) (_Area) { .start = (void *)st, .end = (void *)ed }
static inline int in_range(void *ptr, _Area area) {
  return area.start <= ptr && ptr < area.end;
}

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


void irq0();
void irq1();
void irq14();
void vec0();
void vec1();
void vec2();
void vec3();
void vec4();
void vec5();
void vec6();
void vec7();
void vec8();
void vec9();
void vec10();
void vec11();
void vec12();
void vec13();
void vec14();
void vecsys();
void irqall();

#endif
