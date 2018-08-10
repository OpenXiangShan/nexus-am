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
int _asye_init(_Context *(*handler)(_Event, _Context *)) {
  trace_wrapper(int, _asye_init, asye_init, (handler), 1, handler);
}
_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  trace_wrapper(_Context *, _kcontext, kcontext, (stack, entry, arg), 2, entry, arg);
}
void _yield() {
  trace_wrapper_noret(int, _yield, yield, (), 1, 0);
}
int _intr_read() {
  trace_wrapper(int, _intr_read, intr_read, (), 1, 0);
}
void _intr_write(int enable) {
  trace_wrapper_noret(int, _intr_write, intr_write, (enable), 1, enable);
}
_Context *_irq_callback(_Event ev, _Context *ctx) {
  trace_wrapper(_Context *, _irq_callback, irq_callback, (ev, ctx), 4, ev.event, ev.cause, ev.ref, ctx);
}
#undef  TRACE_THIS

#define TRACE_THIS _TRACE_PTE
int _pte_init(void * (*pgalloc_f)(size_t), void (*pgfree_f)(void *)) {
  trace_wrapper(int, _pte_init, pte_init, (pgalloc_f, pgfree_f), 2, pgalloc_f, pgfree_f) ;
}
int _map(_Protect *p, void *va, void *pa, int prot) {
  trace_wrapper(int, _map, map, (p, va, pa, prot), 4, p, va, pa, prot);
}
_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {
  trace_wrapper(_Context *, _ucontext, ucontext, (p, ustack, kstack, entry, args), 3, p, entry, args);
}
void _switch(_Protect *p) {
  trace_wrapper_noret(int, _switch, vm_switch, (p), 1, p);
}
int _protect(_Protect *p) {
  trace_wrapper(int, _protect, protect, (p), 1, p);
}
void _unprotect(_Protect *p) {
  trace_wrapper_noret(int, _unprotect, unprotect, (p), 1, p);
}
#undef  TRACE_THIS