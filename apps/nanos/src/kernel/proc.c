#include <am.h>
#include <klib.h>
#include "pcb.h"
#include "irq.h"

PCB PCBs[NR_PCBS];
PCB *current;
LIST_HEAD(readyq_h);
LIST_HEAD(freeq_h);
LIST_HEAD(blockq_h);
u16 pcb_avls = 0;	//pcb available index
_Area stack;

void init_idle(){
  PCB *pcb = &(PCBs[pcb_avls]);
  current = pcb;
  current->pid = 0;
  current->sf = (void *)current->p_stack;
  current->state = READY;
  current->time_count = 0;
  current->sleep_time = 0;
  current->lock_depth = 0;
  list_add(&(current->state_list),&readyq_h);
  pcb_avls++;
  strcpy(current->name,"idle");
  _listen(handle);
}

PCB *create_kthread(size_t entry){
  assert(pcb_avls < NR_PCBS);
  PCB *pcb = &(PCBs[pcb_avls]);
  pcb->pid = pcb_avls;	//use index to define pid
  pcb->lock_depth = 0;

  //set space to restore trap_frame
  stack.start = pcb->p_stack;
  stack.end = pcb->p_stack + STACK_SIZE;
  pcb->sf = _make(stack,(void *)entry);
  pcb->time_count = time_chips;
  pcb->sleep_time = 0;

  pcb->state = READY;
  strcpy(pcb->name,"PCB0");
  pcb_avls++;
  list_add(&(pcb->state_list),&readyq_h);
  return pcb;
}

int sys_fork(){
  assert(pcb_avls < NR_PCBS);
  PCB *pcb = &(PCBs[pcb_avls]);
  pcb->pid = pcb_avls;	//use index to define pid
  pcb->lock_depth = 0;

  pcb->sf = (void *)pcb->p_stack;
  memcpy(pcb->sf,current->sf,sizeof(struct _RegSet));
  pcb->time_count = time_chips;
  pcb->sleep_time = 0;

  pcb->state = READY;
  strcpy(pcb->name,"PCB1");
  pcb_avls++;
  list_add(&(pcb->state_list),&readyq_h);
  return 0;
}
