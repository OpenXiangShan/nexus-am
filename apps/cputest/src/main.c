#include <am.h>

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
  while (1);
}
