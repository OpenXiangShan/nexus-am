#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include <am.h>
#include <klib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MB * 1024 * 1024
#define KB * 1024

#define REF_CPU    "i7-6700 @ 3.40GHz"
#define REF_SCORE  100000

// TODO: checksum is little-endian

// the list of benchmarks
//       Name        |  Mem  |Ref(us)| Enable | Checksum | Desc
#define BENCHMARK_LIST(def) \
  def(queen, "queen",   0 KB,  5490, true, 0x00003778, "Queen placement") \
  def(   bf,    "bf",  32 KB, 17520, true, 0x432858fd, "Brainf**k interpreter") \
  def(  fib,   "fib", 256 KB, 28380, true, 0xebdc5f80, "Fibonacci number") \
  def(sieve, "sieve",   2 MB, 41070, true, 0x000a2403, "Eratosthenes sieve") \
  def( 15pz,  "15pz",   8 MB,  5800, true, 0x00068b8c, "A* 15-puzzle search") \
  def(dinic, "dinic",   1 MB, 14750, true, 0x0000a5b9, "Dinic's maxflow algorithm") \
  def( lzip,  "lzip",   4 MB,  8580, true, 0xbf337956, "Lzip compression") \
  def(ssort, "ssort",   4 MB,  4970, true, 0xd40eb177, "Suffix sort") \
  def(  md5,   "md5",  16 MB, 20200, true, 0xa2579514, "MD5 digest") \

// Each benchmark will run REPEAT times
#define REPEAT  1

#define DECL(_name, _sname, _mlim, _ref, _en, _cs, _desc) \
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
  u32 checksum;
} Benchmark;

extern Benchmark *current;
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
void bench_srand(i32 seed);
i32 bench_rand(); // return a random number between 0..32767

// checksum
u32 checksum(void *start, void *end);

#ifdef __cplusplus
}
#endif

#endif
