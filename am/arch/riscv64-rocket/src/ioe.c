#include <am.h>

void _ioe_init() {
}

// -------------------- cycles and uptime --------------------

static unsigned long riscv64_time = 0;

unsigned long _uptime(){
  return riscv64_time ++;
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

int _read_key(){
  return _KEY_NONE;
}

