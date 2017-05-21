#include <am.h>
#include <klib.h>
#include "pcb.h"

int main(){
  _ioe_init();
  _asye_init();
  init_idle();
  create_uthread(entry);
  _ienable();
  while(1){
    _idle();
  }
  return 0;
}
