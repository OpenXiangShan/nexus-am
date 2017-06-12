#include<klib.h>
void other_test(){
  printk("hello,there is other_test\n");
  char buf[128];
  sprintf(buf, "%s", "Hello world!\n");
  assert(strcmp(buf, "Hello world!\n") == 0);
  sprintf(buf, "%d + %d = %d\n", 1, 1, 2);
  assert(strcmp(buf, "1 + 1 = 2\n") == 0);
  sprintf(buf, "%d + %d = %d\n", 2, 10, 12);
  assert(strcmp(buf, "2 + 10 = 12\n") == 0);
  printk("bye,other_test is over here\n");
}
