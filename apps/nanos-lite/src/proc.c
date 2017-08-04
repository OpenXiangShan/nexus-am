#include "proc.h"

#define MAX_NR_PROC 4

PCB pcb[MAX_NR_PROC];
PCB *current = NULL;
int nr_proc = 0;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _make(stack, (void *)entry, NULL);
}

_RegSet* schedule(_RegSet *prev) {
  Log("schedule");
  // when current == NULL at the very beginning, it will not cover
  // any valid data, so it will be safe to write to memory near NULL
  current->tf = prev;
  current = &pcb[0];
  _switch(&current->as);
  return current->tf;
}
