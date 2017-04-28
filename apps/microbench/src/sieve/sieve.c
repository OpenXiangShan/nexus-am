#include <benchmark.h>

#define N 10000000
#define ANS 664579

static int ans;
static u32 *primes;

static inline int get(int n) {
  return (primes[n >> 5] >> (n & 31)) & 1;
}

static inline void clear(int n) {
  primes[n >> 5] &= ~(1ul << (n & 31));
}

void bench_sieve_prepare() {
  primes = (u32*)bench_alloc(N / 8 + 128);
  for (int i = 0; i <= N / 32; i ++) {
    primes[i] = 0xffffffff;
  }
}

void bench_sieve_run() {
  for (int i = 1; i <= N; i ++)
    if (!get(i)) return;
  for (int i = 2; i * i <= N; i ++) {
    if (get(i)) {
      for (int j = i + i; j <= N; j += i)
        clear(j);
    }
  }
  ans = 0;
  for (int i = 2; i <= N; i ++) 
    if (get(i)) {
      ans ++;
    }
}

const char * bench_sieve_validate() {
  return ans == ANS ? NULL : "wrong answer";
}
