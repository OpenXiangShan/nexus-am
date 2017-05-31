#include <am.h>
#include <klib.h>
#include "irq.h"
#include "pcb.h"

void do_syscall(_Event ev, _RegSet *r){
  int *args = ev.cause;
  switch(args[0]){
    case 20:args[0] = sys_fork();break;
    default:_halt(args[0]);
  }
}

_RegSet* handle(_Event ev, _RegSet *r){
  switch(ev.event){
    case _EVENT_IRQ_TIME: {
      memcpy(current->sf, r, sizeof(struct _RegSet));
      if(current->time_count == 0){
        schedule();
      }
      else{
        _putc('-');
        current->time_count--;
      }
      return current->sf;
    }
    case _EVENT_SYSCALL:do_syscall(ev, r);return r;
    default:_halt(ev.event);return r;
  }
}
