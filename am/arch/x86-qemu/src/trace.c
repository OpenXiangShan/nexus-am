#include <am-x86.h>

volatile uint32_t trace_flags = 0;

void _trace_on(uint32_t flags) {
  // TODO: multiprocessor safety
  trace_flags |= flags;
}

void _trace_off(uint32_t flags) {
  trace_flags &= ~flags;
}

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

#define CALL_ARGS(_0, _1, _2, _3, ...) \
  (_CallArgs) { .a0 = ((uintptr_t)_0), \
                .a1 = ((uintptr_t)_1), \
                .a2 = ((uintptr_t)_2), \
                .a3 = ((uintptr_t)_3), } \

#define trace_wrapper_noret(rettype, stub, func, arglist, n, ...) \
  trace_call(stub, CALL_ARGS(__VA_ARGS__, 0, 0, 0, 0)); \
  rettype ret = func arglist; \
  trace_ret(stub, ret);

#define trace_wrapper(rettype, stub, func, arglist, n, ...) \
  trace_wrapper_noret(rettype, stub, func, arglist, n, __VA_ARGS__); \
  return ret;

#define TRACE_NORET(rettype, stub, func, decl, arglist, n, ...) \
  void stub decl { trace_wrapper_noret(int, stub, func, arglist, n, __VA_ARGS__); }

#define TRACE(rettype, stub, func, decl, arglist, n, ...) \
  rettype stub decl { trace_wrapper(rettype, stub, func, arglist, n, __VA_ARGS__); }

#define DECL(tr, rettype, func, decl, args, ...) \
  rettype func decl ;

#define DEF(tr, rettype, func, decl, args, ...) \
  tr(rettype, _##func, func, decl, args, __VA_ARGS__)
 
#define ASYE_FUNCS(_) \
  _(TRACE, int, asye_init, (_Context *(*handler)(_Event, _Context *)), (handler), 1, handler) \
  _(TRACE, _Context *, kcontext, (_Area stack, void (*entry)(void *), void *arg), (stack, entry, arg), 2, entry, arg) \
  _(TRACE, int, intr_read, (), (), 1, 0) \
  _(TRACE, _Context *, irq_callback, (_Event ev, _Context *ctx), (ev, ctx), 4, ev.event, ev.cause, ev.ref, ctx); \
  _(TRACE_NORET, int, yield, (), (), 1, 0) \
  _(TRACE_NORET, int, intr_write, (int enable), (enable), 1, enable)

#define PTE_FUNCS(_) \
  _(TRACE, int, pte_init, (void * (*pgalloc_f)(size_t), void (*pgfree_f)(void *)), (pgalloc_f, pgfree_f), 2, pgalloc_f, pgfree_f) \
  _(TRACE, int, map, (_Protect *p, void *va, void *pa, int prot), (p, va, pa, prot), 4, p, va, pa, prot) \
  _(TRACE, _Context *, ucontext, (_Protect *p, _Area ustack, _Area kstack, void *entry, void *args), (p, ustack, kstack, entry, args), 3, p, entry, args) \
  _(TRACE_NORET, int, prot_switch, (_Protect *p), (p), 1, p) \
  _(TRACE, int, protect, (_Protect *p), (p), 1, p) \
  _(TRACE_NORET, int, unprotect, (_Protect *p), (p), 1, p)

ASYE_FUNCS(DECL)
PTE_FUNCS(DECL)

#define TRACE_THIS _TRACE_ASYE
ASYE_FUNCS(DEF)
#undef  TRACE_THIS

#define TRACE_THIS _TRACE_PTE
PTE_FUNCS(DEF)
#undef  TRACE_THIS
