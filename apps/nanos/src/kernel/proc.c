#include <am.h>
#include <klib.h>
#include "pcb.h"

PCB PCBs[NR_PCBS];
PCB *current;
LIST_HEAD(readyq_h);
LIST_HEAD(freeq_h);
LIST_HEAD(blockq_h);
u16 pcb_avls = 0;	//pcb available index

extern void schedule();

_RegSet* handle(int irq, _RegSet *r){
  _halt(irq);
  return r;
}

void init_idle(){
  PCB *pcb = &(PCBs[pcb_avls]);
  current = pcb;
  //first thread not need to save regs
  current->pid = 0;
  current->state = READY;
  current->time_count = 0;
  current->sleep_time = 0;
  current->lock_depth = 0;
  current->sf = (void *)(pcb->p_stack);
  //the first thread is ready, now add to ready queue
  list_add(&(current->state_list),&readyq_h);
  pcb_avls++;
  strcpy(current->name,"idle");
  _listen(handle);
}

PCB *create_uthread(u32 entry){
  assert(pcb_avls < NR_PCBS);
  PCB *pcb = &(PCBs[pcb_avls]);
  pcb->pid = pcb_avls;	//use index to define pid
  pcb->lock_depth = 0;

  //set space to restore trap_frame
  _Area stack;
  stack.start = pcb->p_stack;
  stack.end = pcb->p_stack + STACK_SIZE;
  pcb->sf = stack.start;
  _RegSet *new_stack = _make(stack,(void *)((size_t)entry));
  memcpy(pcb->sf, new_stack, sizeof(struct _RegSet));
  pcb->time_count = time_chips;
  pcb->sleep_time = 0;

  pcb->state = READY;
  strcpy(pcb->name,"PCB");
  pcb_avls++;
  list_add(&(pcb->state_list),&readyq_h);
  return pcb;
}
