#include <benchmark.h>

#define N 12
#define FULL ((1 << N) - 1)

static int dfs(int row, int ld, int rd) {
  if (row == FULL) {
    return 1;
  } else {
    int pos = FULL & (~(row | ld | rd)), ans = 0;
    while (pos) {
      int p = (pos & (~pos + 1));
      pos -= p;
      ans += dfs(row | p, (ld | p) << 1, (rd | p) >> 1);
    }
    return ans;
  }
}

static int ans;

void bench_queen_prepare() {
  ans = 0;
}

void bench_queen_run() {
  ans = dfs(0, 0, 0);
}

const char * bench_queen_validate() {
  return (ans == current->checksum) ? NULL : "wrong answer";
}
