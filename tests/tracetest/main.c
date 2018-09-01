#include <am.h>
#include <amtrace.h>
#include <klib.h>

void print(const char *s) {
  for (; *s; s ++) {
    _putc(*s);
  }
}

_Context *ihandle(_Event ev, _Context *ctx) {
  if (ev.event == _EVENT_IRQ_IODEV) {
    printf("Received an I/O device interrupt\n");
  }
  return ctx;
}

uintptr_t st;

static void *alloc(size_t size) {
  while (st % size != 0) st++;
  void *ret = (void *)st;
  st += size;
  return ret;
}

static void free(void *ptr) {
}

_Protect prot;

int main() {
  st = (uintptr_t)_heap.start;
  _trace_on(_TRACE_CTE | _TRACE_FUNC);
  _cte_init(ihandle);
  _vme_init(alloc, free);
  _trace_on(_TRACE_VME);
  printf(">>> vme init done.\n");

  uint8_t *ptr = (void*)((uintptr_t)(prot.area.start) +
     ((uintptr_t)(prot.area.end) - (uintptr_t)(prot.area.start)) / 2);

  int pgsz = 4096;

  _map(&prot, ptr, alloc(pgsz), _PROT_WRITE);

  printf(">>> no longer trace function returns\n");
  _trace_off(_TRACE_RET);

  _map(&prot, ptr + pgsz, alloc(pgsz), _PROT_WRITE);

  static char kstk[4096];
  _Area k = { .start = kstk, .end = kstk + 4096 };
  _Area u = { .start = ptr + pgsz, .end = ptr + pgsz * 2 };

  _ucontext(&prot, u, k, ptr, (void*)-1);

  _unprotect(&prot);

  _yield();
  _intr_write(1);
  _trace_off(_TRACE_CTE);

  while (1) ;
  return 1;
}
