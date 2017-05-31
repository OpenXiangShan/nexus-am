#ifndef __PCB_H__
#define __PCB_H__

#include <am.h>
#include <klib.h>
#include "list.h"

#define NR_PCBS 3
#define SLEEP 1
#define READY 0
#define FREE 2
#define time_chips 10
#define STACK_SIZE 4096

typedef struct p_task_table{
  struct _RegSet *sf;
  u32 pid;
  volatile long state;
  int time_count;
  int sleep_time;
  int lock_depth;
  list_head state_list;
  char name[32];
  u8 p_stack[STACK_SIZE];
} PCB;

extern PCB PCBs[NR_PCBS];
extern PCB *current;
extern list_head readyq_h,blockq_h,freeq_h;
extern u16 pcb_avls;

void init_idle();
PCB *create_kthread(size_t entry);
int sys_fork();
void sys_sleep();
void sys_wakeup(u32 time);
void sys_exit(int status);

#endif
