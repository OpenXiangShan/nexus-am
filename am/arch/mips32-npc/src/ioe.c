#include <am.h>
#include <npc.h>

void _ioe_init() {
}

// -------------------- cycles and uptime --------------------

static ulong npc_time = 0;
static ulong npc_cycles = 0;

ulong _uptime(){
  // time (ms) = HIGH * 1000 * (2^32) / HZ + LOW * 1000 / HZ
  // ** be careful of overflow **
  ulong low = GetCount(0);
  ulong high = GetCount(1);
  npc_time = high * 1000 * ((1ul << 31) / HZ) * 2 + low / (HZ / 1000);
  return npc_time;
}

ulong _cycles(){
  // cycles (K) = ((HIGH << 32) | LOW) / 1024
  u32 low = GetCount(0);
  ulong high = GetCount(1);
  npc_cycles = (high << 22) + (low >> 10); //npc_cycles returns Kcycles
  return npc_cycles;
}

// -------------------- video --------------------

_Screen _screen = {
  .width = SCR_WIDTH,
  .height = SCR_HEIGHT,
};

static u8 *fb = VMEM_ADDR;

void _draw_f(_Pixel *p) {
  int i;
  for(i = 0;i < SCR_SIZE; i++){
    fb[i] = (R(p[i]) & 0xc0) | ((G(p[i]) & 0xf0) >> 2)| ((B(p[i]) & 0xc0) >> 6);
  }
}

void _draw_p(int x, int y, _Pixel p) {
  fb[x + y * _screen.width] = (R(p) & 0xc0) | ((G(p) & 0xf0) >> 2) | ((B(p) & 0xc0) >> 6);
}

void _draw_sync() {
}

// -------------------- keyboard --------------------

static inline int upevent(int e) { return e; }
static inline int downevent(int e) { return e | 0x8000; }

int pre_key = _KEY_NONE;

// TODO: refactor
int _read_key(){
  int key_code = in_byte();
  switch(key_code){
    case 'a':{
      if(pre_key != _KEY_A) { int t = pre_key; pre_key = _KEY_A; return upevent(t);}
      else { pre_key = _KEY_A; return downevent(_KEY_A);}
    }
    case 's':{
      if(pre_key != _KEY_S) { int t = pre_key; pre_key = _KEY_S; return upevent(t);}
      else { pre_key = _KEY_S; return downevent(_KEY_S);}
    }
    case 'w':{
      if(pre_key != _KEY_W) { int t = pre_key; pre_key = _KEY_W; return upevent(t);}
      else { pre_key = _KEY_W; return downevent(_KEY_W);}
    }
    case 'd':{
      if(pre_key != _KEY_D) { int t = pre_key; pre_key = _KEY_D; return upevent(t);}
      else { pre_key = _KEY_D; return downevent(_KEY_D);}
    }
    case 'y':{
      if(pre_key != _KEY_Y) { int t = pre_key; pre_key = _KEY_Y; return upevent(t);}
      else { pre_key = _KEY_Y; return downevent(_KEY_Y);}
    }
    case 'g':{
      if(pre_key != _KEY_G) { int t = pre_key; pre_key = _KEY_G; return upevent(t);}
      else { pre_key = _KEY_G; return downevent(_KEY_G);}
    }
    default:{int t = pre_key; pre_key = _KEY_NONE; return upevent(t);}
  }
}
