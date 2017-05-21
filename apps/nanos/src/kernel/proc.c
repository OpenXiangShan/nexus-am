#include <am.h>
#include <klib.h>
#include "pcb.h"

PCB PCBs[NR_PCBS];
PCB *current;
LIST_HEAD(readyq_h);
LIST_HEAD(freeq_h);
LIST_HEAD(blockq_h);
u16 pcb_avls = 0;	//pcb available index

void init_idle(){
  PCB *pcb = &(PCBs[pcb_avls]);
  current = pcb;
  //first thread not need to save regs
  current->pid = 0;
  current->state = ready;
  current->time_count = 0;
  current->sleep_time = 0;
  current->lock_depth = 0;
  //the first thread is ready, now add to ready queue
  list_add(&(current->state_list),&readyq_h);
  pcb_avls++;
  strcpy(current->name,"idle");
}

PCB *create_uthread(u32 entry){
  assert(pcb_avls < NR_PCBS);
  PCB *pcb = &(PCBs[pcb_avls]);
  pcb->pid = pcb_avls;	//use index to define pid
  pcb->lock_depth = 0;

  //set space to restore trap_frame
  pcb->sf = (struct _RegSet *)(pcb->p_stack + STACK_SIZE) - 1;
  _Area stack;
  stack.start = pcb->sf;
  stack.end = pcb->sf + 1;
  _make(stack,(void *)entry);
  pcb->time_count = time_chips;
  pcb->sleep_time = 0;

  pcb->state = READY;
  strcpy(pcb->name,"PCB");
  pcb_avls++;
  list_add(&(pcb->state_list),&readyq_h);
  return pcb;
}
