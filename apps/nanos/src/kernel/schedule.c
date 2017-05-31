#include <am.h>
#include <klib.h>
#include "pcb.h"

void schedule() {
  if(!(list_empty(&readyq_h)) && (current->state == READY)){
    struct list_head *tmp;
    tmp = current->state_list.next;
    if(tmp == &readyq_h){
      tmp = tmp->next;
    }
    //search next pcb
    int i;
    for(i = 0; PCBs[i].state_list.next != tmp->next && PCBs[i].state_list.prev != tmp->prev; i++);
    PCB *next = (void *)&(PCBs[i]);
    printk("next = %s",next->name);
    //fresh new pcb time count
    if(current->pid != 0){
      current->time_count = time_chips;
    }
    //change current pcb
    current = next;
  }
  else
    current = &PCBs[0];
}
