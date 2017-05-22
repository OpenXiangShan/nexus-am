#include <am.h>
#include <klib.h>

_Protect kernel;
_Protect user;

void *palloc(){
  return (void *)0xc0000000;
}

void pfree(void *addr){
}

void init_kvm(){
  _pte_init(palloc,pfree);
}
