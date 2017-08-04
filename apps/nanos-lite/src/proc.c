#include "memory.h"

#define NR_PROC 4
#define STACK_SIZE (128 * PGSIZE)

static _RegSet* pcb[NR_PROC] = {};
_Protect as[NR_PROC] = {};
static uint8_t stacks[NR_PROC][STACK_SIZE] = {};

uintptr_t loader(_Protect *as);

void load_first_prog() {
  _protect(&as[0]);

  uintptr_t entry = loader(&as[0]);

  _Area stack;
  stack.start = stacks[0];
  stack.end = stack.start + STACK_SIZE;

  pcb[0] = _make(stack, (void *)entry, NULL);
}

_RegSet* schedule() {
  Log("schedule");
  return pcb[0];
}
