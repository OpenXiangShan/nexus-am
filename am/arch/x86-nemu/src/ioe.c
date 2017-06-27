#include <am.h>
#include <x86-nemu.h>

void _ioe_init() {
}

// -------------------- cycles and uptime --------------------

static unsigned long nemu_time = 0;

uintptr_t _uptime(){
  return nemu_time ++;
}

// -------------------- video --------------------


_Screen _screen = {
  .width  = 0,
  .height = 0,
};

void _draw_p(int x, int y, uint32_t p) {
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

