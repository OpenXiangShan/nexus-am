#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

static inline uintptr_t port_read(int port, size_t nmemb) {
  switch (nmemb) {
    case 1: return inb(port);
    case 2: return inw(port);
    case 4: return inl(port);
    default: assert(0); return 0;
  }
}

static inline void port_write(int port, size_t nmemb, uintptr_t data) {
  switch (nmemb) {
    case 1: outb(port, data); break;
    case 2: outw(port, data); break;
    case 4: outl(port, data); break;
    default: assert(0);
  }
}

static inline uintptr_t no_read(uintptr_t reg, size_t nmemb) {
  assert(0);
  return 0;
}

static inline void no_write(uintptr_t reg, size_t nmemb, uintptr_t data) {
  assert(0);
}

// timer

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

static uintptr_t timer_read(uintptr_t reg, size_t nmemb) {
  switch(reg) {
    case _DEV_TIMER_REG_UPTIME: return port_read(RTC_PORT, nmemb) - boot_time;
    default: assert(0); return 0;
  }
}

// input

#define KBD_DATA_PORT 0x60

static uintptr_t input_read(uintptr_t reg, size_t nmemb) {
  switch(reg) {
    case _DEV_INPUT_REG_KBD: return port_read(KBD_DATA_PORT, nmemb);
    default: assert(0); return 0;
  }
}

// video
static int W, H;
uint32_t* const fb = (uint32_t *)0x40000;

static uintptr_t video_read(uintptr_t reg, size_t nmemb) {
  switch(reg) {
    case _DEV_VIDEO_REG_WIDTH: return W;
    case _DEV_VIDEO_REG_HEIGHT: return H;
    default: assert(0); return 0;
  }
}

static void video_write(uintptr_t reg, size_t nmemb, uintptr_t data) {
  static int x, y, w, h;
  static uint32_t *pixels;
  switch(reg) {
    case _DEV_VIDEO_REG_PIXELS: pixels = (uint32_t*)data; break;
    case _DEV_VIDEO_REG_X: x = data; break;
    case _DEV_VIDEO_REG_Y: y = data; break;
    case _DEV_VIDEO_REG_W: w = data; break;
    case _DEV_VIDEO_REG_H: h = data; break;
    case _DEV_VIDEO_REG_DRAW: {
      int len = sizeof(uint32_t) * ( (x + w >= W) ? W - x : w );
      uint32_t *p_fb = &fb[y * W + x];
      for (int j = 0; j < h; j ++) {
        if (y + j < H) { memcpy(p_fb, pixels, len); }
        else { break; }
        p_fb += W;
        pixels += w;
      }
      break;
    }
    case _DEV_VIDEO_REG_SYNC: break;
  }
}

void _ioe_init() {
  boot_time = timer_read(_DEV_TIMER_REG_UPTIME, 4);
  W = 400;
  H = 320;
}

static _Device n86_dev[] = {
  {_DEV_TIMER,   "NEMU Timer", timer_read, no_write},
  {_DEV_INPUT,   "NEMU Keyboard Controller", input_read, no_write},
  {_DEV_VIDEO,   "NEMU VGA Controller", video_read, video_write},
};

#define NR_DEV (sizeof(n86_dev) / sizeof(n86_dev[0]))

_Device *_device(int n) {
  n --;
  return (n >= 0 && (unsigned int)n < NR_DEV) ? &n86_dev[n] : NULL;
}
