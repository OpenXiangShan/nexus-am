#include <am.h>
#include <klib.h>

_Protect kernel;
_Protect user;

void *palloc(){
  return NULL;
}

void pfree(void *addr){
}

void init_kvm(){
  _pte_init(palloc,pfree);
}
