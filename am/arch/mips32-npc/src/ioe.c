#include <am.h>
#include <npc.h>

void _ioe_init() {
  timer_init();
}

// -------------------- cycles and uptime --------------------

static unsigned long npc_time = 0;
static unsigned long npc_cycles = 0;

unsigned long _uptime(){
  //1. Read the upper 32-bit timer/counter register (TCR1).
  //2. Read the lower 32-bit timer/counter register (TCR0).
  //3. Read the upper 32-bit timer/counter register (TCR1) again. If the value is different from
  //the 32-bit upper value read previously, go back to previous step (reading TCR0).
  //Otherwise 64-bit timer counter value is correct. 
  unsigned long TCR1 = get_TCR(1);
  unsigned long TCR0 = 0;
  do {
    TCR0 = get_TCR(0);
  }while(TCR1 != get_TCR(1));
  //50MHZ
  // time (ms) = HIGH * 1000 * (2^32) / HZ + LOW * 1000 / HZ
  // ** be careful of overflow **
  npc_time = TCR1 * 1000 * ((1ul << 31) / HZ) * 2 + TCR0 / (HZ / 1000);
  return npc_time;
}

unsigned long _cycles(){
  // cycles (K) = ((HIGH << 32) | LOW) / 1024
/*  uint32_t low = GetCount(0);
  unsigned long high = GetCount(1);
  npc_cycles = (high << 22) + (low >> 10); //npc_cycles returns Kcycles*/

  unsigned long TCR1 = get_TCR(1);
  unsigned long TCR0 = 0;
  do {
    TCR0 = get_TCR(0);
  }while(TCR1 != get_TCR(1));
  npc_cycles = (TCR1 << 22) + (TCR0 >> 10); //npc_cycles returns Kcycle
  return npc_cycles;
}

// -------------------- video --------------------

_Screen _screen = {
  .width = SCR_WIDTH,
  .height = SCR_HEIGHT,
};

static uint8_t *fb = VMEM_ADDR;

void _draw_p(int x, int y, uint32_t p) {
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

// -------------------- timer --------------------
void timer_init() {
  volatile uint32_t *pTCSR0 = (uint32_t *)TIMER_BASE;
  volatile uint32_t *pTLR0  = (uint32_t *)(TIMER_BASE + 0x4);
  volatile uint32_t *pTCSR1 = (uint32_t *)(TIMER_BASE + 0x10);
  volatile uint32_t *pTLR1  = (uint32_t *)(TIMER_BASE + 0x14);

  //Clear the timer enable bits in control registers (TCSR0 and TCSR1).
  *pTCSR0 = 0x0; 
  *pTCSR1 = 0x0;

  //Write the lower 32-bit timer/counter load register (TLR0).
  *pTLR0 = 0x0;
  *pTLR1 = 0x0;

  //Set the CASC bit in Control register TCSR0.
  *pTCSR0 |= (1 << 11);

  //Set other mode control bits in control register (TCSR0) as needed.
  //up counter, generate mode

  //Enable the timer in Control register (TCSR0).
  *pTCSR0 |= (1 << 7); 

}

uint32_t get_TCR(int sel) {
  volatile uint32_t *pTCR0 = (uint32_t *)(TIMER_BASE + 0x8);
  volatile uint32_t *pTCR1 = (uint32_t *)(TIMER_BASE + 0x18);

  if(sel == 1) {
    return *pTCR1;
  }
  else if(sel == 0) {
    return *pTCR0;
  }
  else {
    _halt(1);
  }

  return 0;
}
