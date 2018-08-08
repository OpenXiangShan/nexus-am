#include <am.h>
#include <amdev.h>
#include <klib.h>

int ntraps = 0;
_RegSet* handler(_Event ev, _RegSet *regs) {
  switch (ev.event) {
    case _EVENT_YIELD:
      break;
    case _EVENT_IRQ_TIMER:
    case _EVENT_IRQ_IODEV: 
      printf(".");
      break;
    case _EVENT_PAGEFAULT:
      printf("PF: %x %s%s%s\n",
        ev.ref,
        (ev.cause & _PROT_NONE) ? "[not present]" : "",
        (ev.cause & _PROT_READ) ? "[read fail]" : "",
        (ev.cause & _PROT_WRITE) ? "[write fail]" : "");
      break;
    default:
      assert(0);
  }
  return regs;
}

static uintptr_t st;

static void *alloc(size_t size) {
  while (st % size != 0) st++;
  void *ret = (void *)st;
  st += size;
  return ret;
}

static void free(void *ptr) {
}

_Protect prot;

int main(){
  st = (uintptr_t)_heap.start;
  _ioe_init();
  _asye_init(handler);
  _pte_init(alloc, free);

  _protect(&prot);
  _switch(&prot);

  _map(&prot, (void*)0x40000000, alloc(4096), _PROT_WRITE);

  volatile uint32_t *ptr = (uint32_t*)0x40000fff;
  *ptr = 1;

  _intr_write(1);
  while (1) {
    _yield();
  }
  return 0;
}
