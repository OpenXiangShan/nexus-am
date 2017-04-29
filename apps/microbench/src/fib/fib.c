#include <benchmark.h>

// f(n) = (f(n-1) + f(n-2) + .. f(n-m)) mod 2^32

#define N 2147483603
#define M 91
#define DIM M

typedef u32 Matrix[DIM][DIM];

static inline void mult(Matrix *c, Matrix *a, Matrix *b) {
  for (int i = 0; i < DIM; i ++)
    for (int j = 0; j < DIM; j ++) {
      (*c)[i][j] = 0;
      for (int k = 0; k < DIM; k ++) {
        (*c)[i][j] += (*a)[i][k] * (*b)[k][j];
      }
    }
}

static inline void assign(Matrix *a, Matrix *b) {
  for (int i = 0; i < DIM; i ++)
    for (int j = 0; j < DIM; j ++)
      (*a)[i][j] = (*b)[i][j];
}

static Matrix *A, *ans, *T, *tmp;

void bench_fib_prepare() {
  A = bench_alloc(sizeof(Matrix));
  T = bench_alloc(sizeof(Matrix));
  ans = bench_alloc(sizeof(Matrix));
  tmp = bench_alloc(sizeof(Matrix));
}

void bench_fib_run() {
  for (int i = 0; i < DIM; i ++)
    for (int j = 0; j < DIM; j ++) {
      (*T)[i][j] = (*A)[i][j] = (i == DIM - 1 || j == i + 1);
      (*ans)[i][j] = (i == j);
    }

  for (int n = N; n > 0; n >>= 1) {
    if (n & 1) {
      mult(tmp, ans, T);
      assign(ans, tmp);
    }
    mult(tmp, T, T);
    assign(T, tmp);
  }
}

const char * bench_fib_validate() {
  return (*ans)[DIM-1][DIM-1] == (u32)current->checksum ? NULL : "wrong answer";
}
