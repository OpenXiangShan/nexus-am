#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

typedef uint32_t size_t;

struct _Context {
  struct _AddressSpace *prot;
  uintptr_t edi, esi, ebp, esp;
  uintptr_t ebx, edx, ecx, eax;      // Register saved by pushal
  int       irq;                     // # of irq
  uintptr_t err, eip, cs, eflags;    // Execution state before trap
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
