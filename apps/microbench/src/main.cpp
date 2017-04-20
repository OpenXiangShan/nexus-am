#include <am.h>
#include <benchmark.h>

#define ENTRY(nm, snm, ml, en) \
  { .prepare = bench_##nm##_prepare, \
    .run = bench_##nm##_run, \
    .name = snm, .mlim = ml, .enabled = en },

Benchmark benchmarks[] = {
  BENCHMARK_LIST(ENTRY)
};

const char *check(Benchmark &bench) {
  if (!bench.enabled) {
    return "not enabled";
  }
  // insufficient heap
  return NULL;
}

int main() {
  // the benchmark driver
  _trm_init();
  _ioe_init();

  Result res;
  
  for (auto &bench: benchmarks) {
    const char *msg = check(bench);
    if (msg != NULL) {
      // printf("Not executed. %s", msg);
      continue;
    }
    ulong tsc = 0, msec = 0;
    for (int i = 0; i < REPEAT; i ++) {
      _putc(bench.name[0]);
      bench.prepare(); // prepare first
      prepare(&res);   // clean everything, start timer
      bench.run();     // run it
      done(&res);      // collect results

      tsc += res.tsc;
      msec += res.msec;
    }

    tsc /= REPEAT;
    msec /= REPEAT; // TODO: handle overflow

    _putc('\n');
  }

  _halt(0);
}
