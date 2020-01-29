#include "x86_64-qemu.h"
#include <amdev.h>

void __am_vga_init();
void __am_timer_init();

#define DEF_DEVOP(fn) \
  size_t fn(uintptr_t reg, void *buf, size_t size);

DEF_DEVOP(__am_input_read);
DEF_DEVOP(__am_timer_read);
DEF_DEVOP(__am_video_read);
DEF_DEVOP(__am_video_write);


static int scan_code[] = {
   0, 1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88,
     41,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 43,
     58, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 28,
     42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
     29, 91, 56, 57, 56, 29, 
     72, 80, 75, 77, 0, 0, 0, 0, 0, 0
};

size_t __am_input_read(uintptr_t reg, void *buf, size_t size) {
  _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;

  int status = inb(0x64);
  kbd->keydown = 0;
  kbd->keycode = _KEY_NONE;

  if ((status & 0x1) == 0) {
  } else {
    if (status & 0x20) { // mouse
    } else {
      int code = inb(0x60) & 0xff;

      for (int i = 0; i < sizeof(scan_code) / sizeof(scan_code[0]); i ++) {
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



static _DEV_TIMER_DATE_t boot_date;
static uint32_t freq_mhz = 2000;
static uint64_t uptsc;

static inline uint64_t rdtsc() {
  uint32_t lo, hi;
  asm volatile ("rdtsc": "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

static inline int read_rtc(int reg) {
  outb(0x70, reg);
  int ret = inb(0x71);
  return (ret & 0xf) + (ret >> 4) * 10;
}

static void read_rtc_async(_DEV_TIMER_DATE_t *rtc) {
  rtc->second = read_rtc(0);
  rtc->minute = read_rtc(2);
  rtc->hour   = read_rtc(4);
  rtc->day    = read_rtc(7);
  rtc->month  = read_rtc(8);
  rtc->year   = read_rtc(9) + 2000;
}

static void wait_sec(_DEV_TIMER_DATE_t *t1) {
  _DEV_TIMER_DATE_t t0;
  while (1) {
    read_rtc_async(&t0);
    for (int volatile i = 0; i < 100000; i++) ;
    read_rtc_async(t1);
    if (t0.second != t1->second) {
      return;
    }
  }
}

static uint32_t estimate_freq() {
  _DEV_TIMER_DATE_t rtc1, rtc2;
  uint64_t tsc1, tsc2, t1, t2;

  wait_sec(&rtc1);
  tsc1 = rdtsc();
  t1 = rtc1.hour * 3600 + rtc1.minute * 60 + rtc1.second;
  wait_sec(&rtc2);
  tsc2 = rdtsc();
  t2 = rtc2.hour * 3600 + rtc2.minute * 60 + rtc2.second;
  if (t1 >= t2) return estimate_freq(); // passed a day; try again
  return ((tsc2 - tsc1) >> 20) / (t2 - t1);
}

static void get_date(_DEV_TIMER_DATE_t *rtc) {
  int tmp;
  do {
    read_rtc_async(rtc);
    tmp         = read_rtc(0);
  } while (tmp != rtc->second);
}

void __am_timer_init() {
  freq_mhz = estimate_freq();
  get_date(&boot_date);
  uptsc = rdtsc();
}

size_t __am_timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      uint64_t tsc = rdtsc() - uptsc;
      uint32_t mticks = (tsc >> 20);
      uint32_t ms = mticks * 1000 / freq_mhz;
      _DEV_TIMER_UPTIME_t *uptime = (_DEV_TIMER_UPTIME_t *)buf;
      uptime->hi = 0;
      uptime->lo = ms;
      return sizeof(_DEV_TIMER_UPTIME_t);
    }
    case _DEVREG_TIMER_DATE: {
      get_date((_DEV_TIMER_DATE_t *)buf);
      return sizeof(_DEV_TIMER_DATE_t);
    }
  }
  return 0;
}


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

static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}
static uint8_t R(uint32_t p) { return p >> 16; }
static uint8_t G(uint32_t p) { return p >> 8; }
static uint8_t B(uint32_t p) { return p; }

static struct FBPixel {
  uint8_t b, g, r;
} __attribute__ ((packed)) *fb;
typedef struct FBPixel FBPixel;
static int W, H;

void __am_vga_init() {
  VBEInfo *info = (VBEInfo *)0x00004000;
  W = info->width;
  H = info->height;
  fb = upcast(info->framebuffer);
}

size_t __am_video_read(uintptr_t reg, void *buf, size_t size) {
  switch(reg) {
    case _DEVREG_VIDEO_INFO: {
      _DEV_VIDEO_INFO_t *info = (_DEV_VIDEO_INFO_t *)buf;
      info->width = W;
      info->height = H;
      return sizeof(_DEV_VIDEO_INFO_t);
    }
  }
  return 0;
}

size_t __am_video_write(uintptr_t reg, void *buf, size_t size) {
  switch(reg) {
    case _DEVREG_VIDEO_FBCTRL: {
      _DEV_VIDEO_FBCTRL_t *ctl = (_DEV_VIDEO_FBCTRL_t *)buf;
      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      int len = (x + w >= W) ? W - x : w;
      FBPixel *v;
      for (int j = 0; j < h; j ++) {
        if (y + j < H) {
          v = &fb[x + (j + y) * W];
          for (int i = 0; i < len; i ++, v ++) {
            uint32_t p = pixels[i];
            v->r = R(p); v->g = G(p); v->b = B(p);
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

size_t __am_storage_read(uintptr_t reg, void *buf, size_t size) {
  switch(reg) {
    case _DEVREG_STORAGE_INFO: {
      _DEV_STORAGE_INFO_t *info = (void *)buf;
      info->blksz = 512;
      info->blkcnt = 524288;
      return sizeof(*info);
    }
  }
  return 0;
}

static inline void wait_disk(void) {
  while ((inb(0x1f7) & 0xc0) != 0x40);
}

static inline void read_sect(void *buf, uint32_t sect, uint32_t remain) {
}

size_t __am_storage_write(uintptr_t reg, void *buf, size_t size) {
  _DEV_STORAGE_RDCTRL_t *ctl = (void *)buf;
  int is_read = 0;
  switch(reg) {
    case _DEVREG_STORAGE_RDCTRL:
      is_read = 1;
      break;
    case _DEVREG_STORAGE_WRCTRL:
      break;
    default:
      return 0;
  }

  uint32_t blkno = ctl->blkno, remain = ctl->blkcnt;
  uint32_t *ptr = ctl->buf;
  for (remain = ctl->blkcnt; remain; remain--, blkno++) {
    wait_disk();
    outb(0x1f2, 1);
    outb(0x1f3, blkno);
    outb(0x1f4, blkno >> 8);
    outb(0x1f5, blkno >> 16);
    outb(0x1f6, (blkno >> 24) | 0xe0);
    outb(0x1f7, is_read ? 0x20 : 0x30);
    wait_disk();
    if (is_read) {
      for (int i = 0; i < 512 / 4; i ++) {
        *ptr++ = inl(0x1f0);
      }
    } else {
      for (int i = 0; i < 512 / 4; i ++) {
        outl(0x1f0, *ptr++);
      }
    }
  }
  return sizeof(*ctl);
}

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_INPUT:   return __am_input_read(reg, buf, size);
    case _DEV_TIMER:   return __am_timer_read(reg, buf, size);
    case _DEV_VIDEO:   return __am_video_read(reg, buf, size);
    case _DEV_STORAGE: return __am_storage_read(reg, buf, size);
  }
  return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_VIDEO:   return __am_video_write(reg, buf, size);
    case _DEV_STORAGE: return __am_storage_write(reg, buf, size);
  }
  return 0;
}

int _ioe_init() {
  if (_cpu() != 0) panic("init IOE in non-bootstrap CPU");
  __am_timer_init();
  __am_vga_init();
  return 0;
}
