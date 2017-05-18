#include <hello.h>
#include <am.h>
//#include "../../../klib/klib.h"
#include<klib.h>

int main() {
  _trm_init();
  _ioe_init();
  char src[10]="hello";
  //char dst[10];
  //strcpy(dst,src);
  //strncpy(dst,src,5);
  memcpy(src+1,src,5);
  //strcat(dst,src);
  //sprintf(dst,"%d%s%c",1024,src,'t');
  //printk("%d%s",memcmp(src,dst,6),src);
  printk("%3d",1);
  assert(1==0);
  //for (int i = 0; i < 10; i ++) {
  //  print("Hello World!\n");
}
