#include <am.h>
#include <klib.h>
#include "pcb.h"

extern int umain();
extern void init_kvm();
u32 entry = (u32)umain;

int main(){
  _ioe_init();
  init_kvm();
  /*_asye_init();
  init_idle();
  create_uthread(entry);
  _ienable();
  while(1){
    _idle();
  }*/
  _halt(0);
  return 0;
}
