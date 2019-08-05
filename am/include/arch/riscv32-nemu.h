#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  union {
    struct _AddressSpace *prot;
    uint32_t gpr[32];
  };
  uint32_t cause;
  uint32_t status;
  uint32_t epc;
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10] // a0
#define GPR3 gpr[11] // a1
#define GPR4 gpr[12] // a2
#define GPRx gpr[10] // a0

#endif
