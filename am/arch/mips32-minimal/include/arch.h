#ifndef __ARCH_H__
#define __ARCH_H__

typedef unsigned int u32;
typedef int i32;
typedef unsigned short u16;
typedef short int i16;
typedef unsigned char u8;
typedef char i8;
typedef u32 size_t;

typedef struct _RegSet {
} _RegSet;

static inline void _halt(int code) { while (1); }
static inline void _putc(char ch) { } 

#define good_trap(cond) \
  do { \
    if ((cond)) _halt(0); else _halt(1); \
  } while (0)

#endif
