#include "../x86-qemu.h"

volatile uint32_t trace_flags = 0; // current trace status
LOCKDEF(trace);

void _trace_on(uint32_t flags) {
  trace_lock();
  trace_flags |= flags;
  trace_unlock();
}

void _trace_off(uint32_t flags) {
  trace_lock();
  trace_flags &= ~flags;
  trace_unlock();
}

static inline void log_byte(uint8_t ch) {
  #define COM2 0x2f8
  while ((inb(COM2 + 5) & 0x20) == 0) ;
  outb(COM2, ch);
}

static inline void log_ev(_TraceEvent ev, int length, void *ptr) {
  trace_lock();
  for (int i = 0; i < sizeof(ev); i++) {
    log_byte(((char *)&ev)[i]);
  }
  for (int i = 0; i < length; i++) {
    log_byte(((char *)ptr)[i]);
  }
  trace_unlock();
}

#define EV(_type, _ref) \
  (_TraceEvent) { \
    .cpu = (uint16_t)_cpu(), \
    .type = _type, \
    .time = 0, \
    .ref = (uintptr_t)_ref }

#define TRACE_CALL(fn, args) \
  do { \
    uint32_t flags = trace_flags; \
    if (SHOULD_TRACE(flags, _TRACE_CALL)) { \
      log_ev(EV(_TRACE_CALL, fn), sizeof(args), &args); \
    } \
  } while (0)

#define TRACE_RET(fn, retval) \
  do { \
    uint32_t flags = trace_flags; \
    if (SHOULD_TRACE(flags, _TRACE_RET)) { \
      uintptr_t val = (uintptr_t)retval; \
      log_ev(EV(_TRACE_RET, fn), sizeof(uintptr_t), &val); \
    } \
  } while (0)

// ========================= trace wrappers ==========================

#define SHOULD_TRACE(flags, require) \
  (((flags) & TRACE_THIS) && ((flags) & (require)))

#define TRACE_ARGS(_0, _1, _2, _3, ...) \
  (_CallArgs) { .a0 = ((uintptr_t)_0), \
                .a1 = ((uintptr_t)_1), \
                .a2 = ((uintptr_t)_2), \
                .a3 = ((uintptr_t)_3), } \

#ifndef TRACE_DISABLE
  // add TRACE_CALL/TRACE_RET before/after AM function calls
  #define TRACE_VOID(rettype, func, decl, arglist, ...) \
    void _##func decl { \
      TRACE_CALL(func, TRACE_ARGS(__VA_ARGS__, 0, 0, 0, 0)); \
      func arglist; \
      TRACE_RET(func, 0); \
    }

  #define TRACE_FUNC(rettype, func, decl, arglist, ...) \
    rettype _##func decl { \
      TRACE_CALL(func, TRACE_ARGS(__VA_ARGS__, 0, 0, 0, 0)); \
      rettype ret = func arglist; \
      TRACE_RET(func, ret); \
      return ret; \
    }
#else
  // TRACE_DISABLE defined, no tracing (saves some checking time)
  #define TRACE_VOID(rettype, func, decl, arglist, ...) \
    void _##func decl { func arglist; }

  #define TRACE_FUNC(rettype, func, decl, arglist, ...) \
    rettype _##func decl { return func arglist; }
#endif

#define DECL(tr, rettype, func, decl, args, ...) \
  rettype func decl ;

#define DEF(tr, rettype, func, decl, args, ...) \
  TRACE_##tr(rettype, func, decl, args, __VA_ARGS__)

// each function @f to be traced:
//   VF | RT | ARGS (w type) | ARGS (w/o type) | ARGS (log)...
//   (1) VF (void/func): whether @f is void
//   (2) RT (return type): void; int; void *; ...
//   (3) ARGS (arguments w type): (int x, int y); (void *ptr); (); ...
//   (4) ARGS (arguments w/o type): (x, y); (ptr); (); ...
//   (5) ARGS (the arguments to appear in the trace log): x, y, ptr

#define CTE_TRACE_FUNCS(_) \
  _(FUNC,        int,   cte_init, \
    (_Context *(*f)(_Event, _Context *)), (f), f) \
  _(FUNC, _Context *,    kcontext, \
    (_Area stk, void (*f)(void *), void *arg), (stk, f, arg), f, arg) \
  _(VOID,       void,       yield, (), (), 0) \
  _(FUNC,        int,   intr_read, (), (), 0) \
  _(VOID,       void,  intr_write, (int enable), (enable), enable) \
  _(FUNC, _Context *,     _cb_irq, \
    (_Event ev, _Context *ctx), (ev, ctx), ev.event, ev.cause, ev.ref, ctx);

#define VME_TRACE_FUNCS(_) \
  _(FUNC,        int,    vme_init,\
    (void *(*al)(size_t), void (*fr)(void *)), (al, fr), al, fr) \
  _(FUNC,        int,     protect, (_AddressSpace *p), (p), p) \
  _(VOID,       void,   unprotect, (_AddressSpace *p), (p), p) \
  _(FUNC,        int, map, \
    (_AddressSpace *p, void *va, void *pa, int prot), \
    (p, va, pa, prot), p, va, pa, prot) \
  _(FUNC, _Context *,    ucontext, \
    (_AddressSpace *p, _Area us, _Area ks, void *f, void *args), \
    (p, us, ks, f, args), p, f, args) \
  _(FUNC,     void *,   _cb_alloc, (size_t size), (size), size) \
  _(VOID,       void,   _cb_free, (void *ptr), (ptr), ptr)

// ========== real definitions are generated below ==========

CTE_TRACE_FUNCS(DECL)
VME_TRACE_FUNCS(DECL)

#define TRACE_THIS _TRACE_CTE
CTE_TRACE_FUNCS(DEF)
#undef  TRACE_THIS

#define TRACE_THIS _TRACE_VME
VME_TRACE_FUNCS(DEF)
#undef  TRACE_THIS
