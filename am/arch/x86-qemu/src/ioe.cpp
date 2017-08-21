#include <am.h>
#include <x86.h>

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

static void vga_init() {
  VBEInfo *info = reinterpret_cast<VBEInfo*>(0x00004000);
  _screen.width = info->width;
  _screen.height = info->height;
  fb = reinterpret_cast<FBPixel*>(info->framebuffer);
}

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int len = (x + w >= _screen.width) ? _screen.width - x : w;
  FBPixel *v;
  for (int j = 0; j < h; j ++) {
    if (y + j < _screen.height) {
      v = &fb[x + (j + y) * _screen.width];
      for (int i = 0; i < len; i ++, v ++) {
        uint32_t p = pixels[i];
        v->r = R(p); v->g = G(p); v->b = B(p);
      }
    }
    pixels += w;
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

int _read_key() {
  int status = inb(0x64);
  if ((status & 0x1) == 0) return upevent(_KEY_NONE);
  if (status & 0x20) {
    // mouse
    return upevent(_KEY_NONE);
  } else {
    int code = inb(0x60) & 0xff;

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

void _ioe_init() {
  vga_init();
}

unsigned long i386_uptime = 0;

unsigned long _uptime() {
  // TODO: this is not precise.
  return i386_uptime ++;
}

_Screen _screen;

static intptr_t port_read(int port, size_t nmemb) {
  switch (nmemb) {
    case 1: return inb(port);
    case 2: return inw(port);
    case 4: return inl(port);
  }
  return 0;
}

static void port_write(int port, size_t nmemb, intptr_t data) {
  switch (nmemb) {
    case 1: return outb(port, data);
    case 2: return outw(port, data);
    case 4: return outl(port, data);
  }
}

#include <klib.h>

static intptr_t pci_conf_read(intptr_t reg, size_t nmemb) {
  outl(0xcf8, reg);
  switch (nmemb) {
    case 1: return inb(0xcfc + (reg & 3));
    case 2: return inw(0xcfc + (reg & 2));
    case 4: return inl(0xcfc);
  }
  return 0;
}

static void pci_conf_write(intptr_t reg, size_t nmemb, intptr_t data) {
  outl(0xcf8, reg);
  switch (nmemb) {
    case 1: outb(0xcfc + (reg & 3), data);
    case 2: outw(0xcfc + (reg & 2), data);
    case 4: outl(0xcfc, data);
  }
}

static intptr_t hd_read(intptr_t reg, size_t nmemb) {
  return port_read(0x1f0 + reg, nmemb);
}

static void hd_write(intptr_t reg, size_t nmemb, intptr_t data) {
  port_write(0x1f0 + reg, nmemb, data);
}


static _Device x86_dev[] = {
  { 0x00010001, "PCI Configuration", pci_conf_read, pci_conf_write },
  { 0x00010002, "Primary Parallel ATA Hard Disk Controller", hd_read, hd_write },
  { 0x00000000, nullptr, nullptr, nullptr },
};

_Device *_devices = x86_dev;

}
