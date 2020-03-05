#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  union {
    void *pdir;
    uint32_t gpr[32];
  };
  uint32_t lo, hi;
  uint32_t cause, status, epc;
};

#define GPR1 gpr[2] // v0
#define GPR2 gpr[4] // a0
#define GPR3 gpr[5] // a1
#define GPR4 gpr[6] // a2
#define GPRx gpr[2] // v0

#endif
