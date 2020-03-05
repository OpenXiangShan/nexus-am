#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  void *cr3;
  uintptr_t edi, esi, ebp, esp;
  uintptr_t ebx, edx, ecx, eax;      // Register saved by pushal
  int       irq;                     // # of irq
  uintptr_t usp;
  uintptr_t eip, cs, eflags;         // Execution state before trap
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif
