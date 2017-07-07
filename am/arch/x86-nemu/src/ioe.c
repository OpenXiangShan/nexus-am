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

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int len = sizeof(uint32_t) * ( (x + w >= _screen.width) ? _screen.width - x : w );
  for (int j = 0; j < h; j ++) {
    if (y + j < _screen.height) {
      memcpy(&fb[(y + j) * W + x], pixels, len);
    }
    pixels += w;
  }
}

void _draw_sync() {
}

// -------------------- keyboard --------------------

#define KEYDOWN_MASK 0x8000
int scan_code[] = {
  0,
  1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88,
  41, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 43,
  58, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 28,
  42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
  29, 91, 56, 57, 56, 29,
  72, 80, 75, 77, 0, 0, 0, 0, 0, 0
};

int _read_key() {
  int status = inb(0x64);
  if ((status & 0x1) == 0) return _KEY_NONE;

  int code = inb(0x60) & 0xff;
  int keydown_mask = (~code & 0x80) << 8;
  code &= 0x7f;

  for (unsigned int i = 0; i < sizeof(scan_code) / sizeof(scan_code[0]); i ++) {
    if (scan_code[i] == 0) continue;
    if (scan_code[i] == code) {
      return i | keydown_mask;
    }
  }
  return _KEY_NONE;
}

