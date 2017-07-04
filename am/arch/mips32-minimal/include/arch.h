#ifndef __ARCH_H__
#define __ARCH_H__

struct _RegSet {
};

#ifdef __cplusplus
extern "C" {
#endif

static inline void _ioe_init() { }
static inline uintptr_t _uptime() { return 0; }
static inline void _draw_p(int x, int y, uint32_t p) {}
static inline void _draw_sync() {}

#ifdef __cplusplus
}
#endif
#endif
