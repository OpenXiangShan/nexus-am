#ifndef __ARCH_H__
#define __ARCH_H__

#include <riscv32.h>

struct _Context {
  union {
    struct _AddressSpace *prot;
    uint32_t gpr[32];
  };
  uint32_t cause;
  uint32_t status;
  uint32_t epc;
};

#define GPR1 gpr[10]
#define GPR2 gpr[11]
#define GPR3 gpr[12]
#define GPR4 gpr[13]
#define GPRx gpr[10]

#define MAP(c, f) c(f)

#define COUNTERS(f) \
  f(cycle) f(time) f(instr) f(imemStall) \
  f(ALUInstr) f(BRUInstr) f(LSUInstr) f(MDUInstr) f(CSRInstr) \
  f(loadInstr) f(loadStall) f(storeStall) f(mmioInstr) \
  f(IcacheHit) f(DcacheHit) \
  f(mulInstr)

#define CNT_IDX(cnt) PERFCNT_##cnt
#define CNT_ENUM_ITEM(cnt) CNT_IDX(cnt),
enum {
  MAP(COUNTERS, CNT_ENUM_ITEM)
  NR_PERFCNT,
};

typedef struct {
  R64 cnts[NR_PERFCNT];
} PerfCntSet;

void __am_perfcnt_read(PerfCntSet *t);
void __am_perfcnt_sub(PerfCntSet *res, PerfCntSet *t1, PerfCntSet *t0);
void __am_perfcnt_add(PerfCntSet *res, PerfCntSet *t1, PerfCntSet *t0);
void __am_perfcnt_show(PerfCntSet *t);
void __am_perfcnt_excel(PerfCntSet *t);

#endif
