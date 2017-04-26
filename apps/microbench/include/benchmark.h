#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include <am.h>
#include <benchlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MB * 1024 * 1024
#define KB * 1024

#define REF_CPU    "i5-4250u-vbox"
#define REF_SCORE  100000

// the list of benchmarks
//       Name        |  Mem  |Ref(us)| Enable |  Desc
#define BENCHMARK_LIST(def) \
  def(qsort, "qsort", 640 KB, 48000, true, "Quick sort") \
  def(queen, "queen",   0 KB, 11000, true, "Queen placement") \
  def(   bf,    "bf",  32 KB, 51000, true, "Brainf**k interpreter") \
  def(  fib,   "fib", 256 KB, 50000, true, "Fibonacci number") \
  def(sieve, "sieve",   2 MB, 73000, true, "Eratosthenes sieve") \
  def( 15pz,  "15pz",   8 MB, 11000, true, "A* 15-puzzle search") \
  def(dinic, "dinic",   1 MB, 24000, true, "Dinic's maxflow algorithm") \

// Each benchmark will run REPEAT times
#define REPEAT  3

#define DECL(_name, _sname, _mlim, _ref, _en, _desc) \
  void bench_##_name##_prepare(); \
  void bench_##_name##_run(); \
  const char* bench_##_name##_validate();

BENCHMARK_LIST(DECL)

typedef struct Benchmark {
  void (*prepare)();
  void (*run)();
  const char* (*validate)();
  const char *name, *desc;
  ulong mlim, ref;
  int enabled;
} Benchmark;

typedef struct Result {
  int pass;
  const char *msg;
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
