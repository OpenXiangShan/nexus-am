#include <am.h>
#include <x86-nemu.h>

void _ioe_init() {
}

// -------------------- cycles and uptime --------------------

static ulong nemu_time = 0;
static ulong nemu_cycles = 0;

ulong _uptime(){
  return nemu_time ++;
}

ulong _cycles(){
  return nemu_cycles ++;
}

// -------------------- video --------------------


_Screen _screen = {
  .width  = 0,
  .height = 0,
};

void _draw_f(_Pixel *p) {
}

void _draw_p(int x, int y, _Pixel p) {
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

