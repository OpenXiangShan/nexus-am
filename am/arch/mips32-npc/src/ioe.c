#include <am.h>
#include <amdev.h>
#include <npc.h>
#include <klib.h>


struct VBEInfo {
  uint16_t attributes;
  uint8_t window_a;
  uint8_t window_b;
  uint16_t granularity;
  uint16_t window_size;
  uint16_t segment_a;
  uint16_t segment_b;
  uint32_t win_func_ptr;
  uint16_t pitch;
  uint16_t width;
  uint16_t height;
  uint8_t w_char;
  uint8_t y_char;
  uint8_t planes;
  uint8_t bpp;
  uint8_t banks;
  uint8_t memory_model;
  uint8_t bank_size;
  uint8_t image_pages;
  uint8_t reserved0;
 
  uint8_t red_mask;
  uint8_t red_position;
  uint8_t green_mask;
  uint8_t green_position;
  uint8_t blue_mask;
  uint8_t blue_position;
  uint8_t reserved_mask;
  uint8_t reserved_position;
  uint8_t direct_color_attributes;
 
  uint32_t framebuffer;
  uint32_t off_screen_mem_off;
  uint16_t off_screen_mem_size;
  uint8_t reserved1[206];
} __attribute__ ((packed));
typedef struct VBEInfo VBEInfo;

typedef uint16_t FBPixel;
static FBPixel *fb;
static int W, H;

static inline uint8_t R(uint32_t p) { return p >> 16; }
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }
FBPixel RGB_M24_to_M12(uint32_t p) {
	return ((R(p) & 0xf0) << 4) | (G(p) & 0xf0) | ((B(p) & 0xf0) >> 4);
}

void vga_init() {
	fb = (FBPixel*)(VMEM_ADDR);
	W = SCR_WIDTH;
	H = SCR_HEIGHT;
}

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch(reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      info->width = SCR_WIDTH;
      info->height = SCR_HEIGHT;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch(reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;
      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      int len = (x + w >= W) ? W - x : w;
      FBPixel *v;
      for (int j = 0; j < h; j ++) {
        if (y + j < H) {
          v = &fb[x + (j + y) * W];
          for (int i = 0; i < len; i ++, v ++) {
            uint32_t p = pixels[i];
			*v = RGB_M24_to_M12(p);
            // v->r = R(p); v->g = G(p); v->b = B(p);
          }
        }
        pixels += w;
      }
      if (ctl->sync) {
        // do nothing, hardware syncs.
      }
      return sizeof(*ctl);
    }
  }
  return 0;
}

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
#define CYCLE_REG 9
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
#if 1
	  uint32_t cr1 = 0;
	  uint32_t cr0 = 0;
	  MFC0(cr1, CYCLE_REG, 1);
	  MFC0(cr0, CYCLE_REG, 0);

	  uint32_t ms = cr1 * 1000 * ((1ul << 31) / HZ) * 2 + cr0 / (HZ / 1000);

	  _UptimeReg *uptime = (_UptimeReg *)buf;
	  uptime->hi = 0;
      uptime->lo = ms;
#else
	  static uint32_t ms = 0;
	  _UptimeReg *uptime = (_UptimeReg *)buf;
	  uptime->hi = ms += 10;
      uptime->lo = ms;
#endif
      return sizeof(_UptimeReg);
    }
    case _DEVREG_TIMER_DATE: {
	  // do nothing
      return sizeof(_RTCReg);
    }
  }
  return 0;
}


// keyboard
static int ascii_to_keycode[256] = {
  ['\e'] = _KEY_ESCAPE,
  ['1'] = _KEY_1, ['2'] = _KEY_2, ['3'] = _KEY_3, ['4'] = _KEY_4,
  ['5'] = _KEY_5, ['6'] = _KEY_6, ['7'] = _KEY_7, ['8'] = _KEY_8,
  ['9'] = _KEY_9, ['0'] = _KEY_0,
  ['A'] = _KEY_A, ['B'] = _KEY_B, ['C'] = _KEY_C, ['D'] = _KEY_D,
  ['E'] = _KEY_E, ['F'] = _KEY_F, ['G'] = _KEY_G, ['H'] = _KEY_H,
  ['I'] = _KEY_I, ['J'] = _KEY_J, ['K'] = _KEY_K, ['L'] = _KEY_L,
  ['M'] = _KEY_M, ['N'] = _KEY_N, ['O'] = _KEY_O, ['P'] = _KEY_P,
  ['Q'] = _KEY_Q, ['R'] = _KEY_R, ['S'] = _KEY_S, ['T'] = _KEY_T,
  ['U'] = _KEY_U, ['V'] = _KEY_V, ['W'] = _KEY_W, ['X'] = _KEY_X,
  ['Y'] = _KEY_Y, ['Z'] = _KEY_Z,
  ['a'] = _KEY_A, ['b'] = _KEY_B, ['c'] = _KEY_C, ['d'] = _KEY_D,
  ['e'] = _KEY_E, ['f'] = _KEY_F, ['g'] = _KEY_G, ['h'] = _KEY_H,
  ['i'] = _KEY_I, ['j'] = _KEY_J, ['k'] = _KEY_K, ['l'] = _KEY_L,
  ['m'] = _KEY_M, ['n'] = _KEY_N, ['o'] = _KEY_O, ['p'] = _KEY_P,
  ['q'] = _KEY_Q, ['r'] = _KEY_R, ['s'] = _KEY_S, ['t'] = _KEY_T,
  ['u'] = _KEY_U, ['v'] = _KEY_V, ['w'] = _KEY_W, ['x'] = _KEY_X,
  ['y'] = _KEY_Y, ['z'] = _KEY_Z,
  [' '] = _KEY_SPACE,  ['`'] = _KEY_GRAVE,
  // ['~'] = _KEY_,
  // ['!'] = _KEY_, ['@'] = _KEY_, ['#'] = _KEY_,
  // ['$'] = _KEY_, ['%'] = _KEY_, ['^'] = _KEY_,
  // ['&'] = _KEY_, ['*'] = _KEY_, ['('] = _KEY_,
  // [')'] = _KEY_, ['_'] = _KEY_, ['+'] = _KEY_,
  ['-'] = _KEY_MINUS, ['='] = _KEY_EQUALS,
  ['['] = _KEY_LEFTBRACKET, [']'] = _KEY_RIGHTBRACKET,
  ['\\'] = _KEY_BACKSLASH, [';'] = _KEY_SEMICOLON,
  [','] = _KEY_COMMA, ['/'] = _KEY_SLASH,
  // ['\''] = _KEY_,
  ['.'] = _KEY_PERIOD,
  // ['{'] = _KEY_, ['}'] = _KEY_,
  // ['|'] = _KEY_, [':'] = _KEY_, ['"'] = _KEY_,
  // ['<'] = _KEY_, ['>'] = _KEY_, ['?'] = _KEY_, 
  ['\n'] = _KEY_RETURN,
  ['\x7f'] = _KEY_DELETE,
};

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  static char code;
  char in_byte();

  _KbdReg *kbd = (_KbdReg *)buf;
  if(code != 0) {
	kbd->keycode = ascii_to_keycode[(int)code];
	code = 0;
	kbd->keydown = 0;
  } else {
	code = in_byte();
	kbd->keycode = ascii_to_keycode[(int)code];
	kbd->keydown = 1;
  }
  return sizeof(*kbd);
}

static _Device mips32_npc_dev[] = {
  {_DEV_INPUT,   "NOOP Keyboard Controller", input_read, NULL},
  {_DEV_TIMER,   "NOOP Fake Timer",   timer_read, NULL},
  {_DEV_VIDEO,   "NoneStandard VGA Controller",  video_read, video_write},
};

_Device *_device(int n) {
  n --;
  if (n >= 0 && (unsigned int)n < sizeof(mips32_npc_dev) / sizeof(mips32_npc_dev[0])) {
    return &mips32_npc_dev[n];
  } else {
    return NULL;
  }
}

int _ioe_init() {
  vga_init();
  return 0;
}
