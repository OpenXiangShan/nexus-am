#include "common.h"

#define NR_PROC 4
#define STACK_SIZE (128 * PGSIZE)

_RegSet* pcb[NR_PROC];

uintptr_t loader(void);

void load_first_prog() {
  uintptr_t entry = loader();

  _Area stack;
  stack.end = _heap.end;
  stack.start = stack.end - STACK_SIZE;

  pcb[0] = _make(stack, (void *)entry, NULL);
}
