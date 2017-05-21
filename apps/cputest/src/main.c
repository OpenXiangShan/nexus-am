#include <am.h>

#include <klib.h>
#define N 30

int a[N];
int main() {
  a[0] = 0;
  a[1] = 1;
  for (int i = 2; i < N; i ++) {
    a[i] = a[i - 1] + a[i - 2];
  }
  good_trap(a[N - 1] == a[N - 2] + a[N - 3]);
  _halt(1);
  printk("%d + %d = %d Hello, World\n", 1, 2, 3);
  // haha
  while (1);
}
