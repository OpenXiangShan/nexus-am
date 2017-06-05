#include <klib.h>

void memory_test() {
  char src[10]="hello";
  char dst[10];
  strcpy(dst,src);
  assert(strcmp(src,dst)==0);
  strncpy(dst,src,5);
  assert(strncmp(src,dst,5)==0);
  memcpy(src+1,src,5);
  assert(strcmp(src+1,"hello")==0);
  strcat(dst,src);
  assert(strcmp(src,"hellohello")==0);
  //sprintf(dst,"%d%s%c",1024,src,'t');
  //printk("%d%s",memcmp(src,dst,6),src);
  //printk("%-8d\n%03d\n%c\n%s\n",15,1,'c',src,src);
}
