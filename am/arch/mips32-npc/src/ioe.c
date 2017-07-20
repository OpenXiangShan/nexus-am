#include <am.h>
#include <npc.h>
#include <klib.h>
void _ioe_init() {
  real_timer_init();
}

// -------------------- cycles and uptime --------------------

static uintptr_t npc_time = 0;
static unsigned long npc_cycles = 0;

uintptr_t _uptime(){
  //1. Read the upper 32-bit timer/counter register (TCR1).
  //2. Read the lower 32-bit timer/counter register (TCR0).
  //3. Read the upper 32-bit timer/counter register (TCR1) again. If the value is different from
  //the 32-bit upper value read previously, go back to previous step (reading TCR0).
  //Otherwise 64-bit timer counter value is correct. 
  uintptr_t counter_reg1 = real_timer_get_counter_reg(1);
  uintptr_t counter_reg0 = 0;
  do {
    counter_reg0 = real_timer_get_counter_reg(0);
  }while(counter_reg1 != real_timer_get_counter_reg(1));
  //50MHZ
  // time (ms) = HIGH * 1000 * (2^32) / HZ + LOW * 1000 / HZ
  // ** be careful of overflow **
  npc_time = counter_reg1 * 1000 * ((1ul << 31) / HZ) * 2 + counter_reg0 / (HZ / 1000);
  return npc_time;
}

unsigned long _cycles(){
  // cycles (K) = ((HIGH << 32) | LOW) / 1024
/*  uint32_t low = GetCount(0);
  unsigned long high = GetCount(1);
  npc_cycles = (high << 22) + (low >> 10); //npc_cycles returns Kcycles*/

  unsigned long counter_reg1 = real_timer_get_counter_reg(1);
  unsigned long counter_reg0 = 0;
  do {
    counter_reg0 = real_timer_get_counter_reg(0);
  }while(counter_reg1 != real_timer_get_counter_reg(1));
  npc_cycles = (counter_reg1 << 22) + (counter_reg0 >> 10); //npc_cycles returns Kcycle
  return npc_cycles;
}

// -------------------- video --------------------

volatile static uint8_t * const fb = VMEM_ADDR;
_Screen _screen = {
  .width = SCR_WIDTH,
  .height = SCR_HEIGHT,
};


static inline uint8_t R(uint32_t p) { return p >> 16; }
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }
static inline uint8_t pixel(uint32_t p){
	return (R(p) & 0xc0) | ((G(p) & 0xf0) >> 2) | ((B(p) & 0xc0) >> 6);
}

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h){
  int limit_h = (y + h >= _screen.height) ? _screen.height - y : h;
  int limit_w = (x + w >= _screen.width) ? _screen.width - x : w;
  volatile uint8_t *p_fb = &fb[y * _screen.width + x];

  for(int i = 0; i < limit_h; i ++){
    for(int j = 0; j < limit_w; j ++){
	    p_fb[j + i * _screen.width] = pixel(pixels[j + i * w]);
	  }
  }
}

void _draw_sync() {
}

// -------------------- keyboard --------------------

static inline int upevent(int e) { return e; }
static inline int downevent(int e) { return e | 0x8000; }
#define _ascci2key(_, __) \
  _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) \
  __('q', Q) __('w', W) __('e', E) __('r', R) __('t', T) __('y', Y) __('u', U) __('i', I) __('o', O) __('p', P) \
  __('a', A) __('s', S) __('d', D) __('f', F) __('g', G) __('h', H) __('j', J) __('k', K) __('l', L) \
  __('z', Z) __('x', X) __('c', C) __('v', V) __('b', B) __('n', N) __('m', M) 
#define _n2k(n) [n + '0'] = _KEY_NAME(n)
#define _a2k(n1, n2) [n1] =  _KEY_NAME(n2)

unsigned char ascii2key[128] = {
  _ascci2key(_n2k, _a2k)
};

int pre_key = _KEY_NONE;
// q w \0 :pre up, q down, q up, w down, none
#define KEYUP   1
#define KEYDOWN 2
#define FIFO_MAX 30
#define fifo_plus(a) ((a + 1) % FIFO_MAX)
unsigned char keyfifo[FIFO_MAX] = {'a', 0};
int fifo_start = 1, fifo_end = 0;
int keystate = KEYDOWN;

int _read_key(){
  int ascii;
  unsigned char curkey;
  while( fifo_plus(fifo_start) != fifo_end && (ascii = in_byte()) != '\0'){
    curkey = ascii2key[ascii];
    keyfifo[fifo_start] = curkey;
    fifo_start = fifo_plus(fifo_start);
  }

  if(fifo_plus(fifo_end) == fifo_start)
    if(keystate == KEYDOWN)
      return _KEY_NONE;

  if(keystate == KEYUP){
    keystate = KEYDOWN;
    return downevent(keyfifo[fifo_end]);
  }
  else{
    int tmp;
    keystate = KEYUP;
    tmp = upevent(keyfifo[fifo_end]);
    fifo_end = fifo_plus(fifo_end);
    return tmp;
  }
}

// -------------------- timer --------------------
// axi timer control/status reg
#define T_CASC  0x800
#define T_ENALL 0x400
#define T_PWMA  0x200
#define T_TINT  0x100
#define T_ENT   0x80
#define T_ENIT  0x40
#define T_LOAD  0x20
#define T_ARHT  0x10
#define T_CAPT  0x8
#define T_GENT  0x4
#define T_UDT   0x2
#define T_MDT   0x1
#define T_NONE  0x0
const int control_reg0 = 0, control_reg1 = 4;
const int load_reg0 = 1,    load_reg1 = 5;
const int counter_reg0 = 2, counter_reg1 = 6;

// real time
void real_timer_init() {
  volatile uint32_t *timer = (uint32_t *)REAL_TIMER_BASE;
  //Clear the timer enable bits in control registers (TCSR0 and TCSR1).
  timer[control_reg0] = T_NONE;
  timer[control_reg1] = T_NONE;

  //Write the lower 32-bit timer/counter load register (TLR0).
  timer[load_reg0] = 0x0;
  timer[load_reg1] = 0x0;

  //Set the CASC bit in Control register TCSR0.
  timer[control_reg0] |= T_CASC;

  //Set other mode control bits in control register (TCSR0) as needed.
  //up counter, generate mode
  timer[control_reg0] |= T_LOAD;
  timer[control_reg1] |= T_LOAD;

  //Enable the timer in Control register (TCSR0).
  timer[control_reg0] &= (0xffffffff ^ T_LOAD);
  timer[control_reg1] &= (0xffffffff ^ T_LOAD);
  timer[control_reg0] |= T_ENT; 

}

uint32_t real_timer_get_counter_reg(int sel) {
  volatile uint32_t *timer = (uint32_t *)REAL_TIMER_BASE;

  if(sel == 1) 
    return timer[counter_reg1];
  else if(sel == 0)
    return timer[counter_reg0];
  else
    _halt(1);

  return 0;
}

// interrupt timer
static void int_timer_init(uint32_t cycle, uint32_t timer_no, int auto_load){
  volatile uint32_t *timer = (uint32_t *)INT_TIMER_BASE;
  int clr, lr;
  if(timer_no == 0){
	  clr = control_reg0;
	  lr  = load_reg0;
  }
  else if(timer_no == 1){
	  clr = control_reg1;
	  lr  = load_reg1;
  }
  timer[clr] = T_NONE;
  timer[lr]  = cycle;

  //load reg -> counter reg 
  uint32_t flags = auto_load ? T_ARHT : 0;
  flags |= (T_ENIT | T_UDT | T_LOAD);
  timer[clr] = flags;

  // run
  flags = timer[clr];
  flags &= (0xffffffff ^ T_LOAD);
  flags |= T_ENT;
  timer[clr] = flags;
}

void int_timer0_init(uint32_t count_down_cycle, int auto_load){
  int_timer_init(count_down_cycle, 0, auto_load);
}

void int_timer1_init(uint32_t count_down_cycle, int auto_load){
  int_timer_init(count_down_cycle, 1, auto_load);
}

void set_int_timer(int timer_no, int enable, int clear_int, int load){
  volatile uint32_t *timer = (uint32_t *)INT_TIMER_BASE;
  int clr = 0;
  if(timer_no == 0)
    clr = control_reg0;
  else if(timer_no == 1)
    clr = control_reg1;

  uint32_t flags = timer[clr];
  if(enable){
    flags |= T_ENT;
    flags &= (0xffffffff ^ T_LOAD);
  }

  if(clear_int)
    flags |= T_TINT;

  if(load)
    flags |= T_LOAD;
  timer[clr] = flags;
}
