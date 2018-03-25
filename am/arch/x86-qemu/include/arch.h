#ifndef __X86_ARCH_H__
#define __X86_ARCH_H__

#include <stdint.h>

typedef int off_t;
typedef int ssize_t;

struct _RegSet {
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
