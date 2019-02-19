#ifndef __ARCH_H__
#define __ARCH_H__

struct _Context {
  struct _AddressSpace *prot;
  uint32_t eax, ebx, ecx, edx,
           esi, edi, ebp, esp3,
           eip, eflags,
           cs, ds, es, ss,
           ss0, esp0;
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif