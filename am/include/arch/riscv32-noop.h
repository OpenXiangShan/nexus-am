#ifndef __ARCH_H__
#include "riscv32-nemu.h"

#define MAP(c, f) c(f)

#define COUNTERS(f) \
  f(cycle) f(time) f(instr)

#define CNT_IDX(cnt) PERFCNT_##cnt
#define CNT_ENUM_ITEM(cnt) CNT_IDX(cnt),
enum {
  MAP(COUNTERS, CNT_ENUM_ITEM)
  NR_PERFCNT,
};

typedef struct {
  union {
    struct { uint32_t lo, hi; };
    int64_t val;
  } cnts[NR_PERFCNT];
} PerfCntSet;

void __am_perfcnt_read(PerfCntSet *t);
void __am_perfcnt_sub(PerfCntSet *res, PerfCntSet *t1, PerfCntSet *t0);
void __am_perfcnt_add(PerfCntSet *res, PerfCntSet *t1, PerfCntSet *t0);
void __am_perfcnt_show(PerfCntSet *t);
void __am_perfcnt_excel(PerfCntSet *t);

#endif
