#include <am.h>
#include <klib.h>

void printk_test();
void memory_test();

int main() {
  // TODO: snprtinf copies @n chars including the final \0
  printk_test();
  memory_test();
  // Finally reaches here.
  assert(1 == 0);
}
