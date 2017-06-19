#include <am.h>
#include <x86.h>

extern "C" {

struct VBEInfo {
  u16 attributes;
  u8 window_a;
  u8 window_b;
  u16 granularity;
  u16 window_size;
  u16 segment_a;
  u16 segment_b;
  u32 win_func_ptr;
  u16 pitch;
  u16 width;
  u16 height;
  u8 w_char;
  u8 y_char;
  u8 planes;
  u8 bpp;
  u8 banks;
  u8 memory_model;
  u8 bank_size;
  u8 image_pages;
  u8 reserved0;
 
  u8 red_mask;
  u8 red_position;
  u8 green_mask;
  u8 green_position;
  u8 blue_mask;
  u8 blue_position;
  u8 reserved_mask;
  u8 reserved_position;
  u8 direct_color_attributes;
 
  u32 framebuffer;
  u32 off_screen_mem_off;
  u16 off_screen_mem_size;
  u8 reserved1[206];
} __attribute__ ((packed));

static inline _Pixel pixel(u8 r, u8 g, u8 b) {
  return (r << 16) | (g << 8) | b;
}
static u8 R(_Pixel p) { return p >> 16; }
static u8 G(_Pixel p) { return p >> 8; }
static u8 B(_Pixel p) { return p; }

static struct FBPixel {
  u8 b, g, r;
} __attribute__ ((packed)) *fb;

static void vga_init() {
  VBEInfo *info = reinterpret_cast<VBEInfo*>(0x00004000);
  _screen.width = info->width;
  _screen.height = info->height;
  fb = reinterpret_cast<FBPixel*>(info->framebuffer);
}

void _draw_p(int x, int y, _Pixel p) {
  FBPixel &v = fb[x + y * _screen.width];
  v.r = R(p);
  v.g = G(p);
  v.b = B(p);
}

void _draw_f(_Pixel *p) {
  int npx = _screen.width * _screen.height;
  for (int i = 0; i < npx; i ++) {
    fb[i].r = R(p[i]);
    fb[i].g = G(p[i]);
    fb[i].b = B(p[i]);
  }
}

void _draw_sync() {
}

static inline int keydown(int e) { return (e & 0x8000) != 0; }
static inline int upevent(int e) { return e; }
static inline int downevent(int e) { return e | 0x8000; }

int scan_code[] = {
  0,
  1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88,
  41, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 43,
  58, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 28,
  42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
  29, 91, 56, 57, 56, 29, 
  72, 80, 75, 77, 0, 0, 0, 0, 0, 0
};

#include <klib.h>
int _read_key() {
  int status = inb(0x64);
  if ((status & 0x1) == 0) return upevent(_KEY_NONE);
  if (status & 0x20) {
    // mouse
    return upevent(_KEY_NONE);
  } else {
    int code = inb(0x60) & 0xff;
    printk("> %d\n", code);

    for (unsigned int i = 0; i < sizeof(scan_code) / sizeof(int); i ++) {
      if (scan_code[i] == 0) continue;
      if (scan_code[i] == code) {
        return downevent(i);
      } else if (scan_code[i] + 128 == code) {
        return upevent(i);
      }
    }
    return _KEY_NONE;
  }
}

static inline ulong rdtsc() {
  u32 lo, hi;
  asm volatile ("rdtsc": "=a"(lo), "=d"(hi)::);
  return (hi << (32 - 10)) | (lo >> 10);
}

static ulong init_tsc;

void _ioe_init() {
  vga_init();
  init_tsc = rdtsc();
}

ulong i386_uptime = 0;

ulong _cycles() {
  return rdtsc() - init_tsc;
}

ulong _uptime() {
  // TODO: this is not precise.
  return i386_uptime ++;
}

_Screen _screen;

}
