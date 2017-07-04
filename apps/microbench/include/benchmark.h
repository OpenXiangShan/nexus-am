#ifndef __BENCHMARK_H__
#define __BENCHMARK_H__

#include <am.h>
#include <klib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MB * 1024 * 1024
#define KB * 1024

#define true 1
#define false 0

#define REF_CPU    "i7-6700 @ 3.40GHz"
#define REF_SCORE  100000


#define SETTING 1

// the list of benchmarks

//                 size |  heap | time |  checksum   
#define QSORT_SM {     100,   1 KB,     0, 0x6a8e89a3}
#define QSORT_LG {  100000, 640 KB,  5519, 0x057fbc15}
#define QUEEN_SM {       8,   0 KB,     0, 0x0000005c}
#define QUEEN_LG {      12,   0 KB,  5159, 0x00003778}
#define    BF_SM {       4,  32 KB,     0, 0xa6f0079e}
#define    BF_LG {     180,  32 KB, 26209, 0x9221e2b3}
#define   FIB_SM {       2,   1 KB,     0, 0x7cfeddf0}
#define   FIB_LG {      91, 256 KB, 28575, 0xebdc5f80}
#define SIEVE_SM {     100,   1 KB,     0, 0x00000019}
#define SIEVE_LG {10000000,   2 MB, 42406, 0x000a2403}
#define  PZ15_SM {       0,   2 MB,     0, 0x00000006}
#define  PZ15_LG {       1,   8 MB,  5792, 0x00068b8c}

#define BENCHMARK_LIST(def) \
  def(qsort, "qsort", QSORT_SM, QSORT_LG, "Quick sort") \
  def(queen, "queen", QUEEN_SM, QUEEN_LG, "Queen placement") \
  def(   bf,    "bf",    BF_SM,    BF_LG, "Brainf**k interpreter") \
  def(  fib,   "fib",   FIB_SM,   FIB_LG, "Fibonacci number") \
  def(sieve, "sieve", SIEVE_SM, SIEVE_LG, "Eratosthenes sieve") \
  def( 15pz,  "15pz",  PZ15_SM,  PZ15_LG, "A* 15-puzzle search") \

/*
  def(dinic, "dinic",   1 MB, 13536, true, 0x0000c248, "Dinic's maxflow algorithm") \
  def( lzip,  "lzip",   4 MB, 26469, true, 0x60953409, "Lzip compression") \
  def(ssort, "ssort",   4 MB,  5915, true, 0x3f9f2439, "Suffix sort") \
  def(  md5,   "md5",  16 MB, 19593, true, 0x1391e488, "MD5 digest") \
*/

// Each benchmark will run REPEAT times
#define REPEAT  1

#define DECL(_name, _sname, _s1, _s2, _desc) \
  void bench_##_name##_prepare(); \
  void bench_##_name##_run(); \
  int bench_##_name##_validate();

BENCHMARK_LIST(DECL)

typedef struct Setting {
  int size;
  unsigned long mlim, ref;
  uint32_t checksum;
} Setting;

typedef struct Benchmark {
  void (*prepare)();
  void (*run)();
  int (*validate)();
  const char *name, *desc;
  Setting settings[2];
} Benchmark;

extern Benchmark *current;
extern Setting *setting;

typedef struct Result {
  int pass;
  unsigned long tsc, msec;
} Result;

void prepare(Result *res);
void done(Result *res);

// memory allocation
void* bench_alloc(size_t size);
void bench_free(void *ptr);
void bench_reset();

// random number generator
void bench_srand(int32_t seed);
int32_t bench_rand(); // return a random number between 0..32767

// checksum
uint32_t checksum(void *start, void *end);

#ifdef __cplusplus
}
#endif

#endif
