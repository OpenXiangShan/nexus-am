#include "x86-qemu.h"
#include <amdev.h>

void __am_vga_init();
void __am_timer_init();

#define DEF_DEVOP(fn) \
  size_t fn(uintptr_t reg, void *buf, size_t size);

DEF_DEVOP(__am_input_read);
DEF_DEVOP(__am_timer_read);
DEF_DEVOP(__am_video_read);
DEF_DEVOP(__am_video_write);


// AM INPUT (keyboard)

static int scan_code[] = {
   0, 1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88,
     41,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 43,
     58, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 28,
     42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
     29, 91, 56, 57, 56, 29,
     72, 80, 75, 77
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

      for (int i = 0; i < LENGTH(scan_code); i ++) {
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

// AM TIMER (based on rdtsc)

static _DEV_TIMER_DATE_t boot_date;
static uint32_t freq_mhz = 2000;
static uint64_t uptsc;

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
  wait_sec(&rtc1); tsc1 = rdtsc(); t1 = rtc1.minute * 60 + rtc1.second;
  wait_sec(&rtc2); tsc2 = rdtsc(); t2 = rtc2.minute * 60 + rtc2.second;
  if (t1 >= t2) return estimate_freq(); // passed an hour; try again
  return ((tsc2 - tsc1) >> 20) / (t2 - t1);
}

static void get_date(_DEV_TIMER_DATE_t *rtc) {
  int tmp;
  do {
    read_rtc_async(rtc);
    tmp = read_rtc(0);
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
      _DEV_TIMER_UPTIME_t *uptime = (_DEV_TIMER_UPTIME_t *)buf;
      uint64_t tsc = rdtsc() - uptsc;
      uint32_t mticks = (tsc >> 20);
      uint32_t ms = mticks * 1000 / freq_mhz;
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

// AM VIDEO

struct vbe_info {
  uint8_t  ignore[18];
  uint16_t width;
  uint16_t height;
  uint8_t  ignore1[18];
  uint32_t framebuffer;
} __attribute__ ((packed));

static inline uint32_t pixel(uint8_t r, uint8_t g, uint8_t b) { return (r << 16) | (g << 8) | b; }
static inline uint8_t R(uint32_t p) { return p >> 16; }
static inline uint8_t G(uint32_t p) { return p >> 8; }
static inline uint8_t B(uint32_t p) { return p; }

struct pixel {
  uint8_t b, g, r;
} __attribute__ ((packed));

struct pixel *fb;
static int W, H;

void __am_vga_init() {
  struct vbe_info *info = (struct vbe_info *)0x00004000;
  W = info->width;
  H = info->height;
  fb = upcast(info->framebuffer);
}

size_t __am_video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
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
  switch (reg) {
    case _DEVREG_VIDEO_FBCTRL: {
      _DEV_VIDEO_FBCTRL_t *ctl = (_DEV_VIDEO_FBCTRL_t *)buf;
      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      int len = (x + w >= W) ? W - x : w;
      for (int j = 0; j < h; j ++, pixels += w) {
        if (y + j < H) {
          struct pixel *px = &fb[x + (j + y) * W];
          for (int i = 0; i < len; i ++, px ++) {
            uint32_t p = pixels[i];
            px->r = R(p); px->g = G(p); px->b = B(p);
          }
        }
      }
      return sizeof(*ctl);
    }
  }
  return 0;
}

// AM STORAGE

size_t __am_storage_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
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

size_t __am_storage_write(uintptr_t reg, void *buf, size_t size) {
  _DEV_STORAGE_RDCTRL_t *ctl = (void *)buf;
  int is_read = 0;
  switch (reg) {
    case _DEVREG_STORAGE_RDCTRL: is_read = 1; break;
    case _DEVREG_STORAGE_WRCTRL:              break;
    default: return 0;
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
  panic_on(_cpu() != 0, "init IOE in non-bootstrap CPU");
  __am_timer_init();
  __am_vga_init();
  return 0;
}

// LAPIC/IOAPIC (from xv6)

#define ID      (0x0020/4)   // ID
#define VER     (0x0030/4)   // Version
#define TPR     (0x0080/4)   // Task Priority
#define EOI     (0x00B0/4)   // EOI
#define SVR     (0x00F0/4)   // Spurious Interrupt Vector
  #define ENABLE     0x00000100   // Unit Enable
#define ESR     (0x0280/4)   // Error Status
#define ICRLO   (0x0300/4)   // Interrupt Command
  #define INIT       0x00000500   // INIT/RESET
  #define STARTUP    0x00000600   // Startup IPI
  #define DELIVS     0x00001000   // Delivery status
  #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
  #define DEASSERT   0x00000000
  #define LEVEL      0x00008000   // Level triggered
  #define BCAST      0x00080000   // Send to all APICs, including self.
  #define BUSY       0x00001000
  #define FIXED      0x00000000
#define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
#define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
  #define X1         0x0000000B   // divide counts by 1
  #define PERIODIC   0x00020000   // Periodic
#define PCINT   (0x0340/4)   // Performance Counter LVT
#define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
#define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
#define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
  #define MASKED     0x00010000   // Interrupt masked
#define TICR    (0x0380/4)   // Timer Initial Count
#define TCCR    (0x0390/4)   // Timer Current Count
#define TDCR    (0x03E0/4)   // Timer Divide Configuration

#define IOAPIC_ADDR  0xFEC00000   // Default physical address of IO APIC
#define REG_ID     0x00  // Register index: ID
#define REG_VER    0x01  // Register index: version
#define REG_TABLE  0x10  // Redirection table base

#define INT_DISABLED   0x00010000  // Interrupt disabled
#define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
#define INT_ACTIVELOW  0x00002000  // Active low (vs high)
#define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)

volatile unsigned int *__am_lapic = NULL;  // Initialized in mp.c
struct IOAPIC {
    uint32_t reg, pad[3], data;
} __attribute__((packed));
typedef struct IOAPIC IOAPIC;

static volatile IOAPIC *ioapic;

static void lapicw(int index, int value) {
  __am_lapic[index] = value;
  __am_lapic[ID];
}

void __am_percpu_initlapic(void) {
  lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
  lapicw(TDCR, X1);
  lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
  lapicw(TICR, 10000000);
  lapicw(LINT0, MASKED);
  lapicw(LINT1, MASKED);
  if (((__am_lapic[VER]>>16) & 0xFF) >= 4)
    lapicw(PCINT, MASKED);
  lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
  lapicw(ESR, 0);
  lapicw(ESR, 0);
  lapicw(EOI, 0);
  lapicw(ICRHI, 0);
  lapicw(ICRLO, BCAST | INIT | LEVEL);
  while(__am_lapic[ICRLO] & DELIVS) ;
  lapicw(TPR, 0);
}

void __am_lapic_eoi(void) {
  if (__am_lapic)
    lapicw(EOI, 0);
}

void __am_lapic_bootap(uint32_t apicid, void *addr) {
  int i;
  uint16_t *wrv;
  outb(0x70, 0xF);
  outb(0x71, 0x0A);
  wrv = (unsigned short*)((0x40<<4 | 0x67));
  wrv[0] = 0;
  wrv[1] = (uintptr_t)addr >> 4;

  lapicw(ICRHI, apicid<<24);
  lapicw(ICRLO, INIT | LEVEL | ASSERT);
  lapicw(ICRLO, INIT | LEVEL);

  for (i = 0; i < 2; i++){
    lapicw(ICRHI, apicid<<24);
    lapicw(ICRLO, STARTUP | ((uintptr_t)addr>>12));
  }
}

static unsigned int ioapicread(int reg) {
  ioapic->reg = reg;
  return ioapic->data;
}

static void ioapicwrite(int reg, unsigned int data) {
  ioapic->reg = reg;
  ioapic->data = data;
}

void __am_ioapic_init(void) {
  int i, maxintr;

  ioapic = (volatile IOAPIC*)IOAPIC_ADDR;
  maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;

  for (i = 0; i <= maxintr; i++){
    ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
    ioapicwrite(REG_TABLE+2*i+1, 0);
  }
}

void __am_ioapic_enable(int irq, int cpunum) {
  ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
  ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
}
