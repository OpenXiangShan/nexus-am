#ifndef __X86_64_QEMU_H__
#define __X86_64_QEMU_H__

#include <x86.h>
#include <klib.h>
#define MAX_CPU 8

void bootcpu_init();
_Area memory_probe();

struct kernel_stack {
  uint8_t stack[8192];
};

static inline void *stack_top(struct kernel_stack *stk) {
  return stk->stack + sizeof(stk->stack);
}

void __am_iret(_Context *ctx);

struct cpu_local {
  _AddressSpace *uvm;
#if __x86_64__
  SegDesc64 gdt[NR_SEG + 1];
  TSS64 tss;
#else
  SegDesc32 gdt[NR_SEG];
  TSS32 tss;
#endif
  struct kernel_stack stack;
  struct kernel_stack irq_stack;
};

#if __x86_64__
struct trap_frame {
  _Context saved_context;
  uint64_t irq, errcode;
  uint64_t rip, cs, rflags, rsp, ss;
};
#else
struct trap_frame {
  _Context saved_context;
  uint32_t irq, errcode;
  uint32_t eip, cs, eflags, esp, ss;
};
#endif

extern volatile uint32_t *__am_lapic;
extern int __am_ncpu;
extern struct cpu_local __am_cpuinfo[MAX_CPU];

#define CPU (&__am_cpuinfo[_cpu()])
#define LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#define RANGE(st, ed) (_Area) { .start = (void *)st, .end = (void *)ed }
#define IN_RANGE(ptr, area) ((area).start <= (ptr) && (ptr) < (area).end)
#define STRINGIFY(s) #s
#define TOSTRING(s) STRINGIFY(s)
static inline void *upcast(uint32_t ptr) {
  return (void *)(uintptr_t)ptr;
}

static inline void puts(const char *s) {
  for (; *s; s++)
    _putc(*s);
}

#define panic_on(cond, s) \
  do { \
    if (cond) { \
      puts("AM Panic: "); puts(s); \
      puts(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n"); \
      _halt(1); \
    } \
  } while (0)

#define panic(s) panic_on(1, s)

#define bug_on(cond) \
  do { \
    if (cond) panic("internal error (likely a bug in AM)"); \
  } while (0)

#define bug() bug_on(1)

// apic utils
void __am_lapic_eoi();
void __am_ioapic_init();
void __am_lapic_bootap(uint32_t cpu, uint32_t address);
void __am_ioapic_enable(int irq, int cpu);

// x86-specific operations
void __am_bootcpu_init();
void __am_percpu_init();
_Area __am_heap_init();
void  __am_lapic_init();
void __am_othercpu_entry();
void __am_percpu_initirq();
void __am_percpu_initgdt();
void __am_percpu_initlapic();
void __am_thiscpu_setstk0(uintptr_t ss, uintptr_t sp);
void __am_stop_the_world();


#endif
