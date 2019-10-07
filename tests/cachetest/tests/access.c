#include "klib.h"

#define N 64
#define CACHE_SIZE (32 * 1024)
int ans[2 * CACHE_SIZE / sizeof(int)] = {0};

int main() {
  int i = 0;
  for (i = 0; i < N; i++)
    ans[i] = i;

  int start = CACHE_SIZE / sizeof(int);
  for (i = 0; i < N; i++)
    ans[start + i] = start + i;

  for (i = 0; i < N; i++) {
    assert(ans[i] == i);
    assert(ans[start + i] == start + i);
  }
  return 0;
}
