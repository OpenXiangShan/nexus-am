#include <am.h>
#include <benchmark.h>

Benchmark *current;

// The benchmark list

#define ENTRY(_name, _sname, _mlim, _ref, _en, _desc) \
  { .prepare = bench_##_name##_prepare, \
    .run = bench_##_name##_run, \
    .validate = bench_##_name##_validate, \
    .name = _sname, \
    .desc = _desc, \
    .mlim = _mlim, \
    .ref = _ref, \
    .enabled = _en, },

Benchmark benchmarks[] = {
  BENCHMARK_LIST(ENTRY)
};

// Running a benchmark
void bench_prepare(Result *res) {
  // TODO: try best to flush cache and other resources
  res->tsc = _cycles();
  res->msec = _uptime();
}

void bench_done(Result *res) {
  res->tsc = _cycles() - res->tsc;
  res->msec = _uptime() - res->msec;
}

const char *bench_check(Benchmark &bench) {
  if (!bench.enabled) {
    return "";
  }
  ulong freesp = (ulong)_heap.end - (ulong)_heap.start;
  if (freesp < bench.mlim) {
    return "(insufficient memory)";
  }
  return nullptr;
}

Result run_once(Benchmark &b) {
  Result res;
  current = &b;
  bench_reset();       // reset malloc state
  current->prepare();  // call bechmark's prepare function
  bench_prepare(&res); // clean everything, start timer
  current->run();      // run it
  bench_done(&res);    // collect results
  res.msg = current->validate();
  res.pass = res.msg == nullptr;
  return res;
}

ulong score(Benchmark &b, ulong tsc, ulong msec) {
  if (msec == 0) return 0;
  return (REF_SCORE / 1000) * b.ref / msec;
}

int main() {
  _trm_init();
  _ioe_init();

  ulong bench_score = 0;
  int pass = true;

  for (auto &bench: benchmarks) {
    const char *msg = bench_check(bench);
    printk("[%s] %s: ", bench.name, bench.desc);
    if (msg != nullptr) {
      printk("Ignored %s\n", msg);
    } else {
      ulong tsc = 0, msec = 0;
      bool succ = true;
      for (int i = 0; i < REPEAT; i ++) {
        Result res = run_once(bench);
        printk(res.pass ? "*" : "X");
        succ &= res.pass;
        tsc += res.tsc;
        msec += res.msec;
      }

      tsc /= REPEAT;
      msec /= REPEAT; // TODO: handle overflow

      if (succ) printk(" Passed.");
      else printk(" Failed.");

      pass &= succ;

      ulong cur = score(bench, tsc, msec);
      printk("\n  avg time: %dK cycles in %d ms [%d]\n", tsc, msec, cur);

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

static char *start;

void* bench_alloc(size_t size) {
  if ((ulong)start % 16 != 0) {
    start = start + 16 - ((ulong)start % 16);
  }
  char *old = start;
  start += size;
  assert(start < _heap.end);
  for (char *p = old; p != start; p ++) *p = '\0';
  ulong use = (ulong)start - (ulong)_heap.start;
  assert(use <= current->mlim);
  return old;
}

void bench_free(void *ptr) {
}

void bench_reset() {
  start = (char*)_heap.start;
}

static long seed = 1;

void bench_srand(int _seed) {
  seed = _seed & 0x7fff;
}

int bench_rand() {
  seed = (seed * 214013L + 2531011L) >> 16;
  return seed & 0x7fff;
}

// FNV hash
u32 checksum(void *start, void *end) {
  const int x = 16777619;
  int hash = 2166136261;
  for (char *p = (char*)start; p != (char*)end; p ++) {
    hash = (hash ^ *p) * x;
  }
  hash += hash << 13;
  hash ^= hash >> 7;
  hash += hash << 3;
  hash ^= hash >> 17;
  hash += hash << 5;
  return hash;
}


void *operator new(size_t size) {
  return bench_alloc(size);
}

void *operator new[](size_t size) {
  return bench_alloc(size);
}

void operator delete(void *ptr) {
}

void operator delete[](void *ptr) {
}

