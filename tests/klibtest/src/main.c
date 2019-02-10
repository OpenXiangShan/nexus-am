#include <am.h>
#include <klib.h>

void printk_test();
void memory_test();

int main() {
  printf("Test start!\n");
  //printk_test();
  memory_test();
  printf("Test end!\n");
  return 0;
}
