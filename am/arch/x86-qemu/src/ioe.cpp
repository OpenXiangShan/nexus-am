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
  
int _read_key() {
  int status = inb(0x64);
  if ((status & 0x1) == 0) return upevent(_KEY_NONE);
  if (status & 0x20) {
    // mouse
    return upevent(_KEY_NONE);
  } else {
    int code = inb(0x60) & 0xff;
    //printk("> %d\n", code);
    // TODO: this is ugly.
    switch (code) {
      case 44: return downevent(_KEY_Z);
      case 172: return upevent(_KEY_Z);
      case 45: return downevent(_KEY_X);
      case 173: return upevent(_KEY_X);
      case 72: return downevent(_KEY_UP);
      case 200: return upevent(_KEY_UP);
      case 80: return downevent(_KEY_DOWN);
      case 208: return upevent(_KEY_DOWN);
      case 75: return downevent(_KEY_LEFT);
      case 203: return upevent(_KEY_LEFT);
      case 77: return downevent(_KEY_RIGHT);
      case 205: return upevent(_KEY_RIGHT);
    }
    return upevent(_KEY_NONE);
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
