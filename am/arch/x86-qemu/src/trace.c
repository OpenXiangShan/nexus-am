#include <am-x86.h>

volatile uint32_t trace_flags = 0;

void _trace_on(uint32_t flags) {
  // TODO: multiprocessor safety
  trace_flags |= flags;
}

void _trace_off(uint32_t flags) {
  trace_flags &= ~flags;
}

#include <klib.h> // TODO: find a better way to write trace logs

#define TRACE_CALL(fn, args) \
  do { \
    uint32_t flags = trace_flags; \
    if (SHOULD_TRACE(flags, _TRACE_CALL)) { \
      printf("[trace] call  " #fn " (%x) with args {%x, %x, %x, %x}\n", (void *)fn, \
        args.a0, args.a1, args.a2, args.a3); \
    } \
  } while (0)

#define TRACE_RET(fn, retval) \
  do { \
    uint32_t flags = trace_flags; \
    if (SHOULD_TRACE(flags, _TRACE_RET)) { \
      printf("[trace]  ret  " #fn " (%x) -> %x\n", (void *)fn, (uintptr_t)retval); \
    } \
  } while (0)

#define SHOULD_TRACE(flags, require) \
  (((flags) & TRACE_THIS) && ((flags) & (require)))

#define TRACE_ARGS(_0, _1, _2, _3, ...) \
  (_CallArgs) { .a0 = ((uintptr_t)_0), \
                .a1 = ((uintptr_t)_1), \
                .a2 = ((uintptr_t)_2), \
                .a3 = ((uintptr_t)_3), } \

#ifndef TRACE_DISABLE
  // add TRACE_CALL/TRACE_RET before/after AM function calls
  #define TRACE_VOID(rettype, func, decl, arglist, n, ...) \
    void _##func decl { \
      TRACE_CALL(func, TRACE_ARGS(__VA_ARGS__, 0, 0, 0, 0)); \
      func arglist; \
      TRACE_RET(func, 0); \
    }

  #define TRACE_FUNC(rettype, func, decl, arglist, n, ...) \
    rettype _##func decl { \
      TRACE_CALL(func, TRACE_ARGS(__VA_ARGS__, 0, 0, 0, 0)); \
      rettype ret = func arglist; \
      TRACE_RET(func, ret); \
      return ret; \
    }
#else
  // TRACE_DISABLE defined, no tracing (saves some checking time)
  #define TRACE_VOID(rettype, func, decl, arglist, n, ...) \
    void _##func decl { func arglist; }

  #define TRACE_FUNC(rettype, func, decl, arglist, n, ...) \
    rettype _##func decl { return func arglist; }
#endif

#define DECL(tr, rettype, func, decl, args, ...) \
  rettype func decl ;

#define DEF(tr, rettype, func, decl, args, ...) \
  TRACE_##tr(rettype, func, decl, args, __VA_ARGS__)

// each function @f to be traced:
//   VF | RT | ARGS (w type) | ARGS (w/o type) | NR | ARGS (log)...
//   (1) VF (void/func): whether @f is void
//   (2) RT (return type): void; int; void *; ...
//   (3) ARGS (arguments w type): (int x, int y); (void *ptr); (); ...
//   (4) ARGS (arguments w/o type): (x, y); (ptr); (); ...
//   (5) NR (length of the variadic list, must be > 0): 1; 2; 3; ...
//   (6) ARGS (the arguments to appear in the trace log): x, y, ptr

#define ASYE_TRACE_FUNCS(_) \
  _(FUNC, int, asye_init, (_Context *(*handler)(_Event, _Context *)), (handler), 1, handler) \
  _(FUNC, _Context *, kcontext, (_Area stack, void (*entry)(void *), void *arg), (stack, entry, arg), 2, entry, arg) \
  _(VOID, void, yield, (), (), 1, 0) \
  _(FUNC, int, intr_read, (), (), 1, 0) \
  _(VOID, void, intr_write, (int enable), (enable), 1, enable) \
  _(FUNC, _Context *, _cb_irq, (_Event ev, _Context *ctx), (ev, ctx), 4, ev.event, ev.cause, ev.ref, ctx); \

#define PTE_TRACE_FUNCS(_) \
  _(FUNC, int, pte_init, (void * (*pgalloc_f)(size_t), void (*pgfree_f)(void *)), (pgalloc_f, pgfree_f), 2, pgalloc_f, pgfree_f) \
  _(FUNC, int, protect, (_Protect *p), (p), 1, p) \
  _(VOID, void, unprotect, (_Protect *p), (p), 1, p) \
  _(VOID, void, prot_switch, (_Protect *p), (p), 1, p) \
  _(FUNC, int, map, (_Protect *p, void *va, void *pa, int prot), (p, va, pa, prot), 4, p, va, pa, prot) \
  _(FUNC, _Context *, ucontext, (_Protect *p, _Area ustack, _Area kstack, void *entry, void *args), (p, ustack, kstack, entry, args), 3, p, entry, args) \
  _(FUNC, void *, _cb_alloc, (size_t size), (size), 1, size) \
  _(VOID, void, _cb_free, (void *ptr), (ptr), 1, ptr) \

// ========== real definitions are generated below ==========

ASYE_TRACE_FUNCS(DECL)
PTE_TRACE_FUNCS(DECL)

#define TRACE_THIS _TRACE_ASYE
ASYE_TRACE_FUNCS(DEF)
#undef  TRACE_THIS

#define TRACE_THIS _TRACE_PTE
PTE_TRACE_FUNCS(DEF)
#undef  TRACE_THIS
