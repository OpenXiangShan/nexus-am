#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  union {
    struct _AddressSpace *prot;
    uint32_t gpr[32];
  };
  uint32_t lo, hi;
  uint32_t cause, status, epc;
};

#define GPR1 gpr[4]
#define GPR2 gpr[5]
#define GPR3 gpr[6]
#define GPR4 gpr[7]
#define GPRx gpr[2]

#endif
