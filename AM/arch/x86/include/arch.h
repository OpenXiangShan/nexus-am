#ifndef __TYPES_H__
#define __TYPES_H__

typedef char i8;
typedef unsigned char u8;
typedef short int i16;
typedef unsigned short int u16;
typedef int i32;
typedef unsigned int u32;
typedef u32 size_t;
typedef u32 off_t;

typedef struct _RegSet {
  u32 eax, ebx, ecx, edx,
      esi, edi, ebp, esp,
      eip, eflags,
      cs, ds, es, ss;
} _RegSet;

#endif
