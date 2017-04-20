#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include <am.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REPEAT  10

// memory usages
#define MB * 1024 * 1024
#define KB * 1024

// the list of benchmarks
//  name
#define BENCHMARK_LIST(V) \
  V(   qsort,    "qsort",    64 MB,  true) \
  V(  rbtree,   "rbtree",  1024 MB,  true) \

#define DECL(name, sname, mlim, enabled) \
  void bench_##name##_prepare(); \
  void bench_##name##_run();

BENCHMARK_LIST(DECL)

typedef struct Benchmark {
  void (*prepare)();
  void (*run)();
  const char *name;
  ulong mlim;
  int enabled;
} Benchmark;

typedef struct Result {
  ulong tsc, msec;
  ulong score;
} Result;

void prepare(Result *res);
void done(Result *res);

#ifdef __cplusplus
}
#endif

#endif
