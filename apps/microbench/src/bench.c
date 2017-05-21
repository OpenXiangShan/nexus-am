#include <am.h>
#include <benchmark.h>
#include <limits.h>

Benchmark *current;
static char *start;


#define ARR_SIZE(a) (sizeof((a)) / sizeof((a)[0]))

// The benchmark list

#define ENTRY(_name, _sname, _mlim, _ref, _en, _cs, _desc) \
  { .prepare = bench_##_name##_prepare, \
    .run = bench_##_name##_run, \
    .validate = bench_##_name##_validate, \
    .name = _sname, \
    .desc = _desc, \
    .mlim = _mlim, \
    .ref = _ref, \
    .enabled = _en, \
    .checksum = _cs, },

Benchmark benchmarks[] = {
  BENCHMARK_LIST(ENTRY)
};

// Running a benchmark
static void bench_prepare(Result *res) {
  res->tsc = _cycles();
  res->msec = _uptime();
}

static void bench_done(Result *res) {
  res->tsc = _cycles() - res->tsc;
  res->msec = _uptime() - res->msec;
}

static const char *bench_check(Benchmark *bench) {
  if (!bench->enabled) {
    return "";
  }
  ulong freesp = (ulong)_heap.end - (ulong)_heap.start;
  if (freesp < bench->mlim) {
    return "(insufficient memory)";
  }
  return NULL;
}

Result run_once(Benchmark *b) {
  Result res;
  current = b;
  bench_reset();       // reset malloc state
  current->prepare();  // call bechmark's prepare function
  bench_prepare(&res); // clean everything, start timer
  current->run();      // run it
  bench_done(&res);    // collect results
  res.msg = current->validate();
  res.pass = res.msg == NULL;
  return res;
}

ulong score(Benchmark *b, ulong tsc, ulong msec) {
  if (msec == 0) return 0;
  return (REF_SCORE / 1000) * b->ref / msec;
}

int main() {
  _ioe_init();

  ulong bench_score = 0;
  int pass = true;

  for (int i = 0; i < ARR_SIZE(benchmarks); i ++) {
    Benchmark *bench = &benchmarks[i];
    const char *msg = bench_check(bench);
    printk("[%s] %s: ", bench->name, bench->desc);
    if (msg != NULL) {
      printk("Ignored %s\n", msg);
    } else {
      ulong tsc = ULONG_MAX, msec = ULONG_MAX;
      int succ = true;
      for (int i = 0; i < REPEAT; i ++) {
        Result res = run_once(bench);
        printk(res.pass ? "*" : "X");
        succ &= res.pass;
        if (res.tsc < tsc) tsc = res.tsc;
        if (res.msec < msec) msec = res.msec;
      }

      if (succ) printk(" Passed.");
      else printk(" Failed.");

      pass &= succ;

      ulong cur = score(bench, tsc, msec);
      printk("\n  min time: %dK cycles in %d ms [%d]\n", tsc, msec, cur);

      bench_score += cur;
    }
  }

  bench_score /= sizeof(benchmarks) / sizeof(benchmarks[0]);
  
  printk("==================================================\n");
  printk("MicroBench %s        %d Marks\n", pass ? "PASS" : "FAIL", bench_score);
  printk("                   vs. %d Marks (%s)\n", REF_SCORE, REF_CPU);

  _halt(0);
  return 0;
}

// Library


void* bench_alloc(size_t size) {
  if ((ulong)start % 16 != 0) {
    start = start + 16 - ((ulong)start % 16);
  }
  char *old = start;
  start += size;
  assert((ulong)_heap.start <= (ulong)start && (ulong)start < (ulong)_heap.end);
  for (char *p = old; p != start; p ++) *p = '\0';
  assert((ulong)start - (ulong)_heap.start <= current->mlim);
  return old;
}

void bench_free(void *ptr) {
}

void bench_reset() {
  start = (char*)_heap.start;
}

static i32 seed = 1;

void bench_srand(i32 _seed) {
  seed = _seed & 0x7fff;
}

i32 bench_rand() {
  seed = (seed * (i32)214013L + (i32)2531011L) >> 16;
  return seed & 0x7fff;
}

// FNV hash
u32 checksum(void *start, void *end) {
  const i32 x = 16777619;
  i32 hash = 2166136261u;
  for (char *p = (char*)start; p + 4 < (char*)end; p += 4) {
    i32 h1 = hash, h2 = hash;
    for (int i = 0; i < 4; i ++) {
      h1 = (h1 ^ p[i]) * x;
      h2 = (h2 ^ p[3 - i]) * x;
    }
    hash = h1 ^ h2;
  }
  hash += hash << 13;
  hash ^= hash >> 7;
  hash += hash << 3;
  hash ^= hash >> 17;
  hash += hash << 5;
  return hash;
}

