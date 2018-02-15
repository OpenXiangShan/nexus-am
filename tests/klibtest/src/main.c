#include <am.h>
#include <klib.h>

void printk_test();
void memory_test();

int main() {
  printk_test();
  memory_test();
  //printk("%03dd\n",0);
  // Finally reaches here.
  int volatile x = 0;
  assert(x != x);
}
