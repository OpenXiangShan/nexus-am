#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include <am.h>
#include <benchlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REPEAT  1

// memory usages
#define MB * 1024 * 1024
#define KB * 1024

// the list of benchmarks
#define BENCHMARK_LIST(V) \
  V(qsort, "qsort",  1 MB,  true, "quick sort") \
  V(queen, "queen",  0 MB,  true, "queen placement problem") \
  V(   bf,    "bf",  1 MB,  true, "branf**k interpreter") \

#define DECL(name, sname, mlim, enabled, desc) \
  void bench_##name##_prepare(); \
  void bench_##name##_run(); \
  const char* bench_##name##_validate();

BENCHMARK_LIST(DECL)

typedef struct Benchmark {
  void (*prepare)();
  void (*run)();
  const char* (*validate)();
  const char *name, *desc;
  ulong mlim;
  int enabled;
} Benchmark;

typedef struct Result {
  ulong tsc, msec;
  ulong score;
} Result;

void prepare(Result *res);
void done(Result *res);

// memory allocation
void* bench_alloc(size_t size);
void bench_free(void *ptr);
void bench_reset();

// random number generator
void srand(int seed);
int rand(); // return a random number between 0..32767

#ifdef __cplusplus
}
#endif

#endif
