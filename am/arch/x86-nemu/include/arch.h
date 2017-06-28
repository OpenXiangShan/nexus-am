#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (128 * 1024 * 1024)

typedef uint32_t size_t;

struct _RegSet {
  uint32_t edi, esi, ebp, esp_;
  uint32_t ebx, edx, ecx, eax;   // Register saved by pushal
  int      irq;                  // # of irq
  uint32_t err, eip, cs, eflags; // Execution state before trap
};

typedef struct _RegSet TrapFrame;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
