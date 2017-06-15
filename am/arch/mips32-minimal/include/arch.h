#ifndef __ARCH_H__
#define __ARCH_H__

typedef u32 size_t;

struct _RegSet {
};

#ifdef __cplusplus
extern "C" {
#endif

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
