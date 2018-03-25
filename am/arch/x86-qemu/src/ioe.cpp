#include <am.h>
#include <amdev.h>
#include <x86.h>

extern "C" { int printf(const char *, ...); }
static uint32_t freq_mhz;
uint64_t uptsc;

static inline uint64_t rdtsc() {
  uint32_t lo, hi;
  asm volatile ("rdtsc": "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

extern "C" {

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

static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) | (g << 8) | b;
}
static uint8_t R(uint32_t p) { return p >> 16; }
static uint8_t G(uint32_t p) { return p >> 8; }
static uint8_t B(uint32_t p) { return p; }

static struct FBPixel {
  uint8_t b, g, r;
} __attribute__ ((packed)) *fb;
static int W, H;

static void vga_init() {
  VBEInfo *info = reinterpret_cast<VBEInfo*>(0x00004000);
  W = info->width;
  H = info->height;
  fb = reinterpret_cast<FBPixel*>(info->framebuffer);
}

static _Dev_Timer_RTC boot_date;
static int read_rtc(int reg) {
  outb(0x70, reg);
  int ret = inb(0x71);
  return (ret & 0xf) + (ret >> 4) * 10;
}

static void wait_sec() {
  while (1) {
    int volatile s1 = read_rtc(0);
    for (int volatile i = 0; i < 1000; i++) ;
    int volatile s2 = read_rtc(0);
    if (s1 != s2) {
      return;
    }
  }
}

static uint32_t estimate_freq() {
  int h, m, s, t1, t2;
  uint64_t tsc1, tsc2;
  wait_sec();
  tsc1 = rdtsc();
  h = read_rtc(4); m = read_rtc(2); s = read_rtc(0);
  t1 = h * 3600 + m * 60 + s;
  wait_sec();
  tsc2 = rdtsc();
  h = read_rtc(4); m = read_rtc(2); s = read_rtc(0);
  t2 = h * 3600 + m * 60 + s;
  if (t1 >= t2) return 0; // passed a day, unlikely to happen

  uint32_t freq = (tsc2 - tsc1) >> 20;
  freq /= (t2 - t1);
  return freq;
}

static void timer_init() {
  uptsc = rdtsc();
  int tmp;
  do {
    boot_date.second = read_rtc(0);
    boot_date.minute = read_rtc(2);
    boot_date.hour   = read_rtc(4);
    boot_date.day    = read_rtc(7);
    boot_date.month  = read_rtc(8);
    boot_date.year   = read_rtc(9) + 2000;
    tmp              = read_rtc(0);
  } while (tmp != boot_date.second);
  freq_mhz = estimate_freq();
}

void _ioe_init() {
  timer_init();
  vga_init();
}

static uintptr_t port_read(int port, size_t nmemb) {
  switch (nmemb) {
    case 1: return inb(port);
    case 2: return inw(port);
    case 4: return inl(port);
  }
  return 0;
}

static void port_write(int port, size_t nmemb, uintptr_t data) {
  switch (nmemb) {
    case 1: return outb(port, data);
    case 2: return outw(port, data);
    case 4: return outl(port, data);
  }
}

static size_t pciconf_read(uintptr_t reg, void *buf, size_t size) {
  outl(0xcf8, reg);
  switch (size) {
    case 1: *(uint8_t *) buf = inb(0xcfc + (reg & 3));
    case 2: *(uint16_t *)buf = inw(0xcfc + (reg & 2));
    case 4: *(uint32_t *)buf = inl(0xcfc);
  }
  return size;
}

static size_t pciconf_write(uintptr_t reg, void *buf, size_t size) {
  outl(0xcf8, reg);
  switch (size) {
    case 1: outb(0xcfc + (reg & 3), *(uint8_t *) buf);
    case 2: outw(0xcfc + (reg & 2), *(uint16_t *)buf);
    case 4: outl(0xcfc            , *(uint32_t *)buf);
  }
  return size;
}

static inline int keydown(int e) { return (e & 0x8000) != 0; }
static inline int upevent(int e) { return e; }
static inline int downevent(int e) { return e | 0x8000; }

static size_t input_read(uintptr_t reg, void *buf, size_t size) {
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

  int status = inb(0x64), ret = _KEY_NONE;
  if ((status & 0x1) == 0) {
    ret = _KEY_NONE;
  } else {
    if (status & 0x20) { // mouse
      ret = upevent(_KEY_NONE);
    } else {
      int code = inb(0x60) & 0xff;

      for (unsigned int i = 0; i < sizeof(scan_code) / sizeof(int); i ++) {
        if (scan_code[i] == 0) continue;
        if (scan_code[i] == code) {
          ret = downevent(i);
          break;
        } else if (scan_code[i] + 128 == code) {
          ret = upevent(i);
          break;
        }
      }
    }
  }
  *(int *)buf = ret;
  return sizeof(int);
}

static size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEV_TIMER_REG_UPTIME: {
      uint64_t tsc = rdtsc() - uptsc;
      uint32_t mticks = (tsc >> 20);
      uint32_t ms = mticks * 1000 / freq_mhz;
      _Dev_Timer_Uptime *uptime = (_Dev_Timer_Uptime *)buf;
      uptime->hi = 0;
      uptime->lo = ms;
      return sizeof(_Dev_Timer_Uptime);
    }
  }
  return 0;
}

static size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch(reg) {
    case _DEV_VIDEO_REG_INFO: {
      _Dev_Video_Info *info = (_Dev_Video_Info *)buf;
      info->width = W;
      info->height = H;
      return sizeof(_Dev_Video_Info);
    }
  }
  return 0;
}

static size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch(reg) {
    case _DEV_VIDEO_REG_FBCTL: {
      _Dev_Video_FBCtl *ctl = (_Dev_Video_FBCtl *)buf;
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
      return sizeof(size);
    }
  }
  return 0;
}

static size_t hd_read(uintptr_t reg, void *buf, size_t size) {
  *(uint32_t *)buf = port_read(0x1f0 + reg, size);
  return 0;
}

static size_t hd_write(uintptr_t reg, void *buf, size_t size) {
  uint32_t data = *((uint32_t *)buf);
  port_write(0x1f0 + reg, size, data);
  return 0;
}

static _Device x86_dev[] = {
  {_DEV_INPUT,   "8279 Keyboard Controller", input_read, nullptr},
  {_DEV_TIMER,   "Dummy Timer", timer_read, nullptr},
  {_DEV_VIDEO,   "Standard VGA Controller", video_read, video_write},
  {_DEV_PCICONF, "PCI Configuration", pciconf_read, pciconf_write},
  {_DEV_ATA0,    "Primary Parallel ATA Hard Disk Controller", hd_read, hd_write},
};

_Device *_device(int n) {
  n --;
  if (n >= 0 && (unsigned int)n < sizeof(x86_dev) / sizeof(x86_dev[0])) {
    return &x86_dev[n];
  } else {
    return nullptr;
  }
}

}
