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

#define GPR1 gpr[10]
#define GPR2 gpr[11]
#define GPR3 gpr[12]
#define GPR4 gpr[13]
#define GPRx gpr[10]

#endif
