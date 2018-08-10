#include <am-x86.h>

volatile uint32_t trace_flags = 0;

void _trace_on(uint32_t flags) {
  // TODO: multiprocessor safety
  trace_flags |= flags;
}

void _trace_off(uint32_t flags) {
  trace_flags &= ~flags;
}

// wrappers of traced functions
// TODO: add others, add pgalloc()/pgfree() callback traces

// ASYE
int asye_init(_Context *(*handler)(_Event, _Context *));
_Context *kcontext(_Area stack, void (*entry)(void *), void *arg);
int yield();
int intr_read();
int intr_write(int enable);
_Context *irq_callback(_Event ev, _Context *ctx);
// PTE
int pte_init(void * (*pgalloc_f)(size_t), void (*pgfree_f)(void *));
int map(_Protect *p, void *va, void *pa, int prot);
_Context *ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args);
int vm_switch(_Protect *p);
int protect(_Protect *p);
int unprotect(_Protect *p);
// MPE

// ==================== wrapper implementations ====================

#define TRACE_THIS _TRACE_ASYE
TRACE(int, _asye_init, asye_init, (_Context *(*handler)(_Event, _Context *)), (handler), 1, handler);
TRACE(_Context *, _kcontext, kcontext, (_Area stack, void (*entry)(void *), void *arg), (stack, entry, arg), 2, entry, arg);
TRACE_NORET(_yield, yield, (), (), 1, 0);
TRACE(int, _intr_read, intr_read, (), (), 1, 0);
TRACE_NORET(_intr_write, intr_write, (int enable), (enable), 1, enable)
TRACE(_Context *, _irq_callback, irq_callback, (_Event ev, _Context *ctx), (ev, ctx), 4, ev.event, ev.cause, ev.ref, ctx);
#undef  TRACE_THIS

#define TRACE_THIS _TRACE_PTE
TRACE(int, _pte_init, pte_init, (void * (*pgalloc_f)(size_t), void (*pgfree_f)(void *)), (pgalloc_f, pgfree_f), 2, pgalloc_f, pgfree_f) ;
TRACE(int, _map, map, (_Protect *p, void *va, void *pa, int prot), (p, va, pa, prot), 4, p, va, pa, prot);
TRACE(_Context *, _ucontext, ucontext, (_Protect *p, _Area ustack, _Area kstack, void *entry, void *args), (p, ustack, kstack, entry, args), 3, p, entry, args);
TRACE_NORET(_switch, vm_switch, (_Protect *p), (p), 1, p)
TRACE(int, _protect, protect, (_Protect *p), (p), 1, p);
TRACE_NORET(_unprotect, unprotect, (_Protect *p), (p), 1, p);
#undef  TRACE_THIS
