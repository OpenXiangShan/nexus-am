#include <benchmark.h>

#define N 100000
#define CHECKSUM 0xf9404918
int *data;

void bench_qsort_prepare() {
  bench_srand(1);

  data = bench_alloc(N * sizeof(int));
  for (int i = 0; i < N; i ++) {
    data[i] = (bench_rand() << 16) | bench_rand();
  }
}

static void swap(int *a, int *b) {
  int t = *a;
  *a = *b;
  *b = t;
}

static void qsort(int *a, int l, int r) {
  if (l < r) {
    int p = a[l], pivot = l, j;
    for (j = l + 1; j < r; j ++) {
      if (a[j] < p) {
        swap(&a[++pivot], &a[j]);
      }
    }
    swap(&a[pivot], &a[l]);
    qsort(a, l, pivot);
    qsort(a, pivot + 1, r);
  }
}

void bench_qsort_run() {
  qsort(data, 0, N);
}

const char * bench_qsort_validate() {
  return checksum(data, data + N) == CHECKSUM ? NULL : "wrong answer";
}
