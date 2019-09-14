#include <am.h>
#include <klib.h>

#define PERFCNT_BASE 0xb00

#define CNT_NAME(cnt) #cnt,
static const char *name [] = {
  MAP(COUNTERS, CNT_NAME)
};

void __am_perfcnt_read(PerfCntSet *set) {
#define READ_LO(cnt) \
  asm volatile("csrr %0, %1" : "=r"(set->cnts[CNT_IDX(cnt)].lo) : "i"(PERFCNT_BASE + CNT_IDX(cnt)));
#define READ_HI(cnt) \
  asm volatile("csrr %0, %1" : "=r"(set->cnts[CNT_IDX(cnt)].hi) : "i"(PERFCNT_BASE + 0x80 + CNT_IDX(cnt)));

  MAP(COUNTERS, READ_LO)
  MAP(COUNTERS, READ_HI)
}

void __am_perfcnt_sub(PerfCntSet *res, PerfCntSet *t1, PerfCntSet *t0) {
  int i;
  for (i = 0; i < NR_PERFCNT; i ++) {
    if (i != CNT_IDX(time)) {
      res->cnts[i].val = t1->cnts[i].val - t0->cnts[i].val;
    }
  }
}

void __am_perfcnt_add(PerfCntSet *res, PerfCntSet *t1, PerfCntSet *t0) {
  int i;
  for (i = 0; i < NR_PERFCNT; i ++) {
    if (i != CNT_IDX(time)) {
      res->cnts[i].val = t1->cnts[i].val + t0->cnts[i].val;
    }
  }
}

void __am_perfcnt_show(PerfCntSet *t) {
  int i;
  for (i = 0; i < NR_PERFCNT; i ++) {
    if (i != CNT_IDX(time)) {
      printf("{%10u, %10u} <- %s\n", t->cnts[i].hi, t->cnts[i].lo, name[i]);
    }
  }
}

void __am_perfcnt_excel(PerfCntSet *t) {
  int i;
  for (i = 0; i < NR_PERFCNT; i ++) {
    if (i != CNT_IDX(time)) {
      if (i == CNT_IDX(cycle)) {
        printf("%u,", t->cnts[i].hi);
      }
      printf("%u,", t->cnts[i].lo);
    }
  }
  printf("\n");
}
