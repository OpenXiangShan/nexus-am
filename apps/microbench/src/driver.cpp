#include <benchmark.h>

void prepare(Result *res) {
  res->tsc = _cycles();
  res->msec = _uptime();
  // try best to flush cache and other resources

  // log timer, tsc
}

void done(Result *res) {
  // get timer, tsc
  // generate report
}
