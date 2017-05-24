#include <am.h>
#include <klib.h>
#include "pcb.h"

extern int umain();
extern void init_kvm();

int main(){
  _ioe_init();
  _asye_init();
  init_idle();
  init_kvm();
  create_kthread((size_t)umain);
  _ienable();
  while(1){
    _idle();
  }
  return 0;
}
