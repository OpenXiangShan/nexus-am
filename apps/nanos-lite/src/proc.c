#include "proc.h"

#define NR_PROC 4

PCB pcb[NR_PROC];
PCB *current;

uintptr_t loader(_Protect *as);

void load_first_prog() {
  _protect(&pcb[0].as);

  uintptr_t entry = loader(&pcb[0].as);

  _Area stack;
  stack.start = pcb[0].stack;
  stack.end = stack.start + sizeof(pcb[0].stack);

  pcb[0].tf = _make(stack, (void *)entry, NULL);
}

_RegSet* schedule() {
  Log("schedule");
  current = &pcb[0];
  _switch(&current->as);
  return pcb[0].tf;
}
