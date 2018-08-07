#include <am.h>
#include <amdev.h>
#include <x86.h>

extern void vga_init();
extern void timer_init();

int _ioe_init() {
  timer_init();
  vga_init();
  return 0;
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

static size_t hd_read(uintptr_t reg, void *buf, size_t size) {
  *(uint32_t *)buf = port_read(0x1f0 + reg, size);
  return size;
}

static size_t hd_write(uintptr_t reg, void *buf, size_t size) {
  uint32_t data = *((uint32_t *)buf);
  port_write(0x1f0 + reg, size, data);
  return size;
}

size_t video_read(uintptr_t reg, void *buf, size_t size);
size_t video_write(uintptr_t reg, void *buf, size_t size);
size_t timer_read(uintptr_t reg, void *buf, size_t size);
size_t input_read(uintptr_t reg, void *buf, size_t size);

static _Device x86_dev[] = {
  {_DEV_INPUT,   "8279 Keyboard Controller", input_read, NULL},
  {_DEV_TIMER,   "RDTSC Timer / CMOS RTC",   timer_read, NULL},
  {_DEV_VIDEO,   "Standard VGA Controller",  video_read, video_write},
  {_DEV_PCICONF, "PCI Configuration",        pciconf_read, pciconf_write},
  {_DEV_ATA0,    "ATA Disk Controller 0",    hd_read, hd_write},
};

_Device *_device(int n) {
  n --;
  if (n >= 0 && (unsigned int)n < sizeof(x86_dev) / sizeof(x86_dev[0])) {
    return &x86_dev[n];
  } else {
    return NULL;
  }
}
