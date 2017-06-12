#include <am.h>
#include "klib.h"

void printk_test();
void memory_test();
void other_test();

int main() {
  printk_test();
  //memory_test();
  other_test();
  // Finally reaches here.
  assert(1 == 0);
}
