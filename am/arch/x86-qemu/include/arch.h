#ifndef __X86_ARCH_H__
#define __X86_ARCH_H__

#include <sys/types.h>
typedef char i8;
typedef unsigned char u8;
typedef short int i16;
typedef unsigned short int u16;
typedef int i32;
typedef unsigned int u32;

typedef struct _RegSet {
  u32 eax, ebx, ecx, edx,
      esi, edi, ebp, esp3,
      eip, eflags,
      cs, ds, es, ss,
      ss0, esp0;
} _RegSet;

#endif
