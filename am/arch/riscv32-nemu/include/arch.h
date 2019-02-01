#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096

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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
