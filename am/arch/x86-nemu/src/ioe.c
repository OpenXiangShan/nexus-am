#include <am.h>
#include <x86-nemu.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static uint32_t boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

// -------------------- cycles and uptime --------------------

uintptr_t _uptime() {
  return inl(RTC_PORT) - boot_time;
  //return 0;
}

// -------------------- video --------------------

uint32_t* const fb = (uint32_t *)0x40000;
#define SCREEN_W 320
#define SCREEN_H 200

_Screen _screen = {
  .width  = SCREEN_W,
  .height = SCREEN_H,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int len = sizeof(uint32_t) * ( (x + w >= _screen.width) ? _screen.width - x : w );
  uint32_t *p_fb = &fb[y * _screen.width + x];
  for (int j = 0; j < h; j ++) {
    if (y + j < _screen.height) {
      memcpy(p_fb, pixels, len);
    }
    else {
      break;
    }
    p_fb += _screen.width;
    pixels += w;
  }
}

void _draw_sync() {
}

// -------------------- keyboard --------------------

#define KEYDOWN_MASK 0x8000

int _read_key() {
  int status = inb(0x64);
  if ((status & 0x1) == 0) return _KEY_NONE;

  int code = inb(0x60) & 0xff;
  int keydown_mask = code & 0x80;
  code &= 0x7f;
  keydown_mask = (keydown_mask && (code != _KEY_NONE) ? KEYDOWN_MASK : 0);

  return code | keydown_mask;
}
