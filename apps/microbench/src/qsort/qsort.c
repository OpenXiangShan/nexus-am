#include <benchmark.h>
#include <benchlib.h>

#define N 10000
int data[N];

void bench_qsort_prepare() {
  srand(1);
  //data = bench_alloc(n * sizeof(int));
  for (int i = 0; i < N; i ++) {
    data[i] = rand();
  }
}

void bench_qsort_run() {
  // TODO: change to qsort, with func pointer
  for (int i = 0; i < N; i ++)
    for (int j = i + 1; j < N; j ++)
      if (data[i] > data[j]) {
        int tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
      }
}

const char * bench_qsort_validate() {
  for (int i = 1; i < N; i ++) {
    if (data[i - 1] > data[i]) return "Order violation";
  }
  return NULL;
}
