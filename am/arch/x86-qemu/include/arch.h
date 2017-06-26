#ifndef __X86_ARCH_H__
#define __X86_ARCH_H__

#include <am.h>

#include <sys/types.h>

struct _RegSet {
  uint32_t eax, ebx, ecx, edx,
      esi, edi, ebp, esp3,
      eip, eflags,
      cs, ds, es, ss,
      ss0, esp0;
};

#define REG1(regs) ((regs)->eax)
#define REG2(regs) ((regs)->ebx)
#define REG3(regs) ((regs)->ecx)
#define REG4(regs) ((regs)->edx)

#endif
