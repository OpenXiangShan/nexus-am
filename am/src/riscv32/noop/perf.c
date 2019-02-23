#include <klib.h>

enum {
  PERFCNT_CYCLE,
  PERFCNT_INSTR,
  NR_PERFCNT,
};

static const char *name [] = {
  "cycle",
  "instruction",
};

typedef union {
  struct {
    uint32_t lo, hi;
  };
  uint64_t val;
} PerfCnt;

typedef struct {
  PerfCnt cnts[NR_PERFCNT];
} PerfCntSet;

static PerfCntSet t0;

static void read_perfcnt(PerfCntSet *set) {
  asm volatile("csrr %0, mcycle"    : "=r"(set->cnts[PERFCNT_CYCLE].lo));
  asm volatile("csrr %0, minstret"  : "=r"(set->cnts[PERFCNT_INSTR].lo));
  asm volatile("csrr %0, mcycleh"   : "=r"(set->cnts[PERFCNT_CYCLE].hi));
  asm volatile("csrr %0, minstreth" : "=r"(set->cnts[PERFCNT_INSTR].hi));
}

static void diff_perfcntset(PerfCntSet *diff, PerfCntSet *t1, PerfCntSet *t0) {
  int i;
  for (i = 0; i < NR_PERFCNT; i ++) {
    diff->cnts[i].val = t1->cnts[i].val - t0->cnts[i].val;
    printf("# %s = {%u, %u}\n", name[i], diff->cnts[i].hi, diff->cnts[i].lo);
  }
}

void init_perfcnt(void) {
  read_perfcnt(&t0);
}

void show_perfcnt(void) {
  PerfCntSet t1, diff;
  read_perfcnt(&t1);
  diff_perfcntset(&diff, &t1, &t0);
}
