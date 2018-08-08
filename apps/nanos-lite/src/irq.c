#include "common.h"

_Context* schedule(_Context *);
_Context* do_syscall(_Context *);

static _Context* do_event(_Event e, _Context* r) {
  _Context *ret = NULL;

  switch (e.event) {
    case _EVENT_IRQ_TIME: ret = schedule(r); break;
    case _EVENT_TRAP: ret = schedule(r); break;
    case _EVENT_SYSCALL: ret = do_syscall(r); break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return ret;
}

void init_irq(void) {
  _asye_init(do_event);
}
