#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  void *uvm;
  uint32_t ds, eax, ebx, ecx, edx,
           esp0, esi, edi, ebp,
           eip, cs, eflags, esp, ss3;
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif
