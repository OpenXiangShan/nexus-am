#include <hello.h>
#include <am.h>
//#include "../../../klib/klib.h"
#include<klib.h>

int main() {
  _trm_init();
  _ioe_init();
  char src[10]="hello\n";
  char dst[10];
  //strcpy(dst,src);
  sprintk(dst,"%d%s%c",1024,src,'t');
  printk("%s",dst);
  assert(1==0);
  //for (int i = 0; i < 10; i ++) {
  //  print("Hello World!\n");
}
