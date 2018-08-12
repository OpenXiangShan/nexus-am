#ifndef __AM_X86_H__
#define __AM_X86_H__

#include <am.h>
#include <amdev.h>
#include <amtrace.h>
#include <x86.h>

extern volatile uint32_t *lapic;
extern int ncpu;
extern volatile uint32_t trace_flags;

// apic utils
void lapic_eoi();
void ioapic_init();
void lapic_bootap(int cpu, uint32_t address);
void ioapic_enable(int irq, int cpu);

// per-cpu x86-specific operations
void percpu_initirq();
void percpu_initgdt();
void percpu_initlapic();
void percpu_initpg();
void thiscpu_setustk(uintptr_t ss0, uintptr_t esp0);
void thiscpu_die() __attribute__((__noreturn__));
void allcpu_halt();

// simple spin locks
#define LOCKDECL(name) \
  void name##_lock(); \
  void name##_unlock();

#define LOCKDEF(name) \
  static volatile intptr_t name##_locked = 0; \
  static int name##_lock_flags[MAX_CPU]; \
  void name##_lock() { \
    name##_lock_flags[_cpu()] = get_efl() & FL_IF; \
    cli(); \
    while (1) { \
      if (0 == _atomic_xchg(&name##_locked, 1)) break; \
      __asm__ volatile ("pause"); \
    } \
  } \
  void name##_unlock() { \
    _atomic_xchg(&name##_locked, 0); \
    if (name##_lock_flags[_cpu()]) sti(); \
  }

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

#endif
