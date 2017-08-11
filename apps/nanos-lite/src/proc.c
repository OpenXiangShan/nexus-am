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

  // TODO: remove the following three lines after you have implemented _umake()
  // _switch(&pcb[i].as);
  //  current = &pcb[i];
  //  ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

static int cnt = 0;
static PCB* game_pcb = &pcb[0];

void change_game() {
  Log("game change!");
  game_pcb = (game_pcb == &pcb[0] ? &pcb[2] : &pcb[0]);
}

_RegSet* schedule(_RegSet *prev) {
  // when current == NULL at the very beginning, it will not cover
  // any valid data, so it will be safe to write to memory near NULL
  current->tf = prev;
  if (current == game_pcb) {
    cnt ++;
    if (cnt == 200) {
      current = &pcb[1];
      cnt = 0;
    }
  }
  else {
    current = game_pcb;
  }
  _switch(&current->as);
  return current->tf;
}
