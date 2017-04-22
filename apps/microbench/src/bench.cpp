#include <am.h>
#include <benchmark.h>
#include <benchlib.h>

Benchmark *current;

// The benchmark list

#define ENTRY(nm, snm, ml, en, des) \
  { .prepare = bench_##nm##_prepare, \
    .run = bench_##nm##_run, \
    .validate = bench_##nm##_validate, \
    .name = snm, .desc = des, .mlim = ml, .enabled = en, },

Benchmark benchmarks[] = {
  BENCHMARK_LIST(ENTRY)
};

// Running a benchmark
void bench_prepare(Result *res) {
  // try best to flush cache and other resources
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
  
  // insufficient heap
  return nullptr;
}

bool run_once(Benchmark &bench, Result &res) {
  current = &bench;
  bench_reset();   // reset malloc state
  bench.prepare(); // call bechmark's prepare function
  bench_prepare(&res);   // clean everything, start timer
  bench.run();     // run it
  bench_done(&res);      // collect results

  const char *msg = bench.validate();
  if (msg == nullptr) { // pass
    return true;
  } else { // fail
    return false;
  }

}

int main() {
  _trm_init();
  _ioe_init();

  for (auto &bench: benchmarks) {
    const char *msg = bench_check(bench);
    printk("[%s] %s: ", bench.name, bench.desc);
    if (msg != nullptr) {
      printk("Ignored %s\n", msg);
      continue;
    }
    ulong tsc = 0, msec = 0;
    bool succ = true;
    for (int i = 0; i < REPEAT; i ++) {
      Result res;
      bool s = run_once(bench, res);
      printk(s ? "*" : "X");
      succ &= s;
      tsc += res.tsc;
      msec += res.msec;
    }

    tsc /= REPEAT;
    msec /= REPEAT; // TODO: handle overflow

    if (succ) {
      printk(" Passed.");
    } else {
      printk(" Failed.");
    }
    printk("\n    Average: %dK cycles in %d ms\n", tsc, msec);
  }

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
  for (char *p = old; p != start; p ++) *p = '\0';
  if (start >= _heap.end) {
    return NULL;
  }
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

void srand(int _seed) {
  seed = _seed & 0x7fff;
}

int rand() {
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
