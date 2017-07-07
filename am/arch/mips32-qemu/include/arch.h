#ifndef __ARCH_H__
#define __ARCH_H__

struct _RegSet {
};

#ifdef __cplusplus
extern "C" {
#endif

static inline void _ioe_init() { }
static inline uintptr_t _uptime() { return 0; }
static inline void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {}
static inline void _draw_sync() {}

#ifdef __cplusplus
}
#endif
#endif
