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
  /*
  unsigned long counter_reg1 = real_timer_get_counter_reg(1);
  unsigned long counter_reg0 = 0;
  do {
    counter_reg0 = real_timer_get_counter_reg(0);
  }while(counter_reg1 != real_timer_get_counter_reg(1));
  
  npc_time = counter_reg1 * 1000 * ((1ul << 31) / HZ) * 2 + counter_reg0 / (HZ / 1000);
  return npc_time;
  */

  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
	  static int ms = 0;
      _UptimeReg *uptime = (_UptimeReg *)buf;
      uptime->hi = 0;
      uptime->lo = ms += 10;
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
static int scan_code[] = {
  0,
  1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88,
  41, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 43,
  58, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 28,
  42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
  29, 91, 56, 57, 56, 29, 
  72, 80, 75, 77, 0, 0, 0, 0, 0, 0
};

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  _KbdReg *kbd = (_KbdReg *)buf;

  int status = 0; // inb(0x64);
  kbd->keydown = 0;
  kbd->keycode = _KEY_NONE;

  if ((status & 0x1) == 0) {
  } else {
    if (status & 0x20) { // mouse
    } else {
      // int code = inb(0x60) & 0xff;
      int code = 0x59;

      for (unsigned int i = 0; i < sizeof(scan_code) / sizeof(int); i ++) {
        if (scan_code[i] == 0) continue;
        if (scan_code[i] == code) {
          kbd->keydown = 1;
          kbd->keycode = i;
          break;
        } else if (scan_code[i] + 128 == code) {
          kbd->keydown = 0;
          kbd->keycode = i;
          break;
        }
      }
    }
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
