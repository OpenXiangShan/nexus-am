#include <am.h>
#include <amdev.h>
#include <klib.h>

int ntraps = 0;
_RegSet* handler(_Event ev, _RegSet *regs) {
  switch (ev.event) {
    case _EVENT_IRQ_TIMER:
      break;
    case _EVENT_IRQ_IODEV: 
      break;
    case _EVENT_YIELD:
      break;
    case _EVENT_PAGEFAULT:
      printf("PF: %s%s%s\n",
        (ev.cause & _PROT_NONE) ? "[not present]" : "",
        (ev.cause & _PROT_READ) ? "[read fail]" : "",
        (ev.cause & _PROT_WRITE) ? "[write fail]" : "");
      break;
  }
  return regs;
}

int main(){
  _ioe_init();
  _asye_init(handler);
  _pte_init(NULL, NULL);

  _intr_write(1);
  while (1) {
    _yield();
  }
  return 0;
}
