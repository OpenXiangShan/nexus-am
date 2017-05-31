#include <am.h>
#include <klib.h>
#include "pcb.h"

extern int umain();

int main(){
  _ioe_init();
  _asye_init();
  init_idle();
  _ienable();
  while(1){
    _putc('^');
    _idle();
  }
  return 0;
}
