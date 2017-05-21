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

#ifdef __cplusplus
extern "C" {
#endif

static inline void _halt(int code) { while (1); }
static inline void _putc(char ch) { } 

static inline void _ioe_init() { }
static inline unsigned long _cycles() { return 0; }
static inline unsigned long _uptime() { return 0; }
static inline void _draw_p(int x, int y, u32 p) {}
static inline void _draw_f(u32 *ps) {}
static inline void _draw_sync() {}

#ifdef __cplusplus
}
#endif
#endif
