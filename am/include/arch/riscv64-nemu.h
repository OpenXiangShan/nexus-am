#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  union {
    struct _AddressSpace *as;
    uint64_t gpr[32];
  };
  uint64_t cause;
  uint64_t status;
  uint64_t epc;
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10] // a0
#define GPR3 gpr[11] // a1
#define GPR4 gpr[12] // a2
#define GPRx gpr[10] // a0

#endif
