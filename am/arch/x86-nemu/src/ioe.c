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

#define fb ((uint32_t *)0x40000)
#define SCREEN_W 320
#define SCREEN_H 200

_Screen _screen = {
  .width  = SCREEN_W,
  .height = SCREEN_H,
};


void _draw_p(int x, int y, uint32_t p) {
  fb[y * SCREEN_W + x] = p;
}

void _draw_sync() {
}

// -------------------- keyboard --------------------

static inline int upevent(int e) { return e; }
static inline int downevent(int e) { return e | 0x8000; }

int pre_key = _KEY_NONE;

// TODO: refactor
int _read_key(){
  return 0;
}

