#ifndef __X86_ARCH_H__
#define __X86_ARCH_H__

#include <am.h>

#include <sys/types.h>

struct _RegSet {
  u32 eax, ebx, ecx, edx,
      esi, edi, ebp, esp3,
      eip, eflags,
      cs, ds, es, ss,
      ss0, esp0;
};

#endif
