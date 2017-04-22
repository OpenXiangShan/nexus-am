#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include <am.h>
#include <benchlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MB * 1024 * 1024
#define KB * 1024

#define SMALL
#define LARGE

// the list of benchmarks
//   Name       | Mem   |  Enable |  Desc                          |
#define BENCHMARK_LIST(def) \
  def(qsort, "qsort", 640 KB,  true, "quick sort") \
  def(queen, "queen",   0 KB,  true, "queen placement") \
  def(   bf,    "bf",  32 KB,  true, "branf**k interpreter") \

// Each benchmark will run REPEAT times
#define REPEAT  3

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

// checksum
u32 checksum(void *start, void *end);

#ifdef __cplusplus
}
#endif

#endif
