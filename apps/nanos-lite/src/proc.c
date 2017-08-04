#include "memory.h"

#define NR_PROC 4
#define STACK_SIZE (128 * PGSIZE)

static _RegSet* pcb[NR_PROC] = {};
_Protect as[NR_PROC] = {};

uintptr_t loader(_Protect *as);

void load_first_prog() {
  _protect(&as[0]);

  uintptr_t entry = loader(&as[0]);

  _Area stack;
  stack.end = _heap.end;
  stack.start = stack.end - STACK_SIZE;

  pcb[0] = _make(stack, (void *)entry, NULL);
}

_RegSet* schedule() {
  Log("schedule");
  return pcb[0];
}
