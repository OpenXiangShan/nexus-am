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
void cpu_initgdt();
void cpu_initpte();
void cpu_setustk(uintptr_t ss0, uintptr_t esp0);

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

// Tracing

#include <klib.h> // TODO: don't do this.

#define should_trace(flags, req) ( \
    (flags & TRACE_THIS) && \
    (flags & req) \
  )

#define trace_call(fn, args) \
  do { \
    uint32_t flags = trace_flags; \
    if (should_trace(flags, _TRACE_CALL)) { \
      printf("[trace] call  " #fn " (%x) with args {%x, %x, %x, %x}\n", (void *)fn, \
        args.a0, args.a1, args.a2, args.a3); \
    } \
  } while (0)

#define trace_ret(fn, retval) \
  do { \
    uint32_t flags = trace_flags; \
    if (should_trace(flags, _TRACE_RET)) { \
      printf("[trace]  ret  " #fn " (%x) -> %x\n", (void *)fn, (uintptr_t)retval); \
    } \
  } while (0)

#define get_0(_0, _1, _2, _3, ...) ((uintptr_t)_0)
#define get_1(_0, _1, _2, _3, ...) ((uintptr_t)_1)
#define get_2(_0, _1, _2, _3, ...) ((uintptr_t)_2)
#define get_3(_0, _1, _2, _3, ...) ((uintptr_t)_3)

#define trace_wrapper(rettype, stub, func, arglist, n, ...) \
  _CallArgs call_args = (_CallArgs) { .a0 = get_0(__VA_ARGS__, 0, 0, 0, 0),  \
                                      .a1 = get_1(__VA_ARGS__, 0, 0, 0, 0),  \
                                      .a2 = get_2(__VA_ARGS__, 0, 0, 0, 0),  \
                                      .a3 = get_3(__VA_ARGS__, 0, 0, 0, 0),  \
                       }; \
  trace_call(stub, call_args); \
  rettype ret = func arglist; \
  trace_ret(stub, ret);

#endif