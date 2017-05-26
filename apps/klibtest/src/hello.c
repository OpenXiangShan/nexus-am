#include <hello.h>
#include <am.h>
//#include "../../../klib/klib.h"
#include<klib.h>

int main() {
  char src[10]="hello\n";
  char dst[10];
  strcpy(dst,src);
  //sprintf(dst,"%d%s%c",1024,src,'t');
  printk("%d%s",atoi("-1020"),dst);
  assert(1==0);
  //for (int i = 0; i < 10; i ++) {
  //  print("Hello World!\n");
}
