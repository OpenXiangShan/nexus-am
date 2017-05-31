#include "klib.h"

int fork(void){
  return _trap(20,0,0,0);
}

void sleep(ulong time){
  _trap(21,0,0,0);
}

void exit(int status){
  _trap(22,status,0,0);
}
