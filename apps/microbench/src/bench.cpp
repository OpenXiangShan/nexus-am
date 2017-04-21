#include <am.h>
#include <benchmark.h>

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
  // get timer, tsc
  // generate report
}

const char *bench_check(Benchmark &bench) {
  if (!bench.enabled) {
    return "not enabled";
  }
  ulong freesp = (ulong)_heap.end - (ulong)_heap.start;
  if (freesp < bench.mlim) {
    return "insufficient memory";
  }
  
  // insufficient heap
  return NULL;
}

bool run_once(Benchmark &bench, Result &res) {
  bench_reset();   // reset malloc state
  bench.prepare(); // call bechmark's prepare function
  bench_prepare(&res);   // clean everything, start timer
  bench.run();     // run it
  bench_done(&res);      // collect results

  const char *msg = bench.validate();
  if (msg == NULL) { // pass
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
    // printk("[%s] %s: ", bench.name, bench.desc);
    if (msg != NULL) {
      // printk("Not executed. %s\n", msg);
      continue;
    }
    ulong tsc = 0, msec = 0;
    bool succ = true;
    for (int i = 0; i < REPEAT; i ++) {
      _putc('0' + i % 10);
      Result res;
      succ &= run_once(bench, res);
      tsc += res.tsc;
      msec += res.msec;
    }

    tsc /= REPEAT;
    msec /= REPEAT; // TODO: handle overflow

    if (succ) {
      _putc('O'); _putc('K');
    } else {
      _putc('N'); _putc('O');
    }

    _putc('\n');
  }

  _halt(0);
}

// Library

static char *start;

void* bench_alloc(size_t size) {
  ulong rem = ((ulong)start) % size;
  if (rem != 0) {
    start = start + size - rem;
  }
  char *old = start;
  // TODO: check bound;
  start += size;
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
