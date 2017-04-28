#include "quicklz.h"
#include <benchmark.h>

#define SIZE (1<<20)
#define CHECKSUM 0xcf252bed

static qlz_state_compress *state;
static char *blk;
static char *compress;
static int len;

void bench_lzip_prepare() {
  bench_srand(1);
  state = bench_alloc(sizeof(qlz_state_compress));
  blk = bench_alloc(SIZE);
  compress = bench_alloc(SIZE + 400);
  for (int i = 0; i < SIZE; i ++) {
    blk[i] = 'a' + bench_rand() % 26;
  }
}

void bench_lzip_run() {
  len = qlz_compress(blk, compress, SIZE, state);
}

const char* bench_lzip_validate() {
  return (checksum(compress, compress + len) == CHECKSUM) ? 
    NULL :
    "wrong answer";
}

