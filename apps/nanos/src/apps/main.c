#include <am.h>
#include <klib.h>

int umain(){
  printk("jump into umain");
  _halt(0);
  return 0;
}
