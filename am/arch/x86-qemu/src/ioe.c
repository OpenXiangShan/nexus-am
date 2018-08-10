#include <am-x86.h>

void vga_init();
void timer_init();

#define devop_def(dev, op) \
  size_t dev##_##op(uintptr_t reg, void *buf, size_t size);

#define devops(_) \
  _(input, read) \
  _(timer, read) \
  _(video, read)   _(video, write) \
  _(pciconf, read) _(pciconf, write) \
  _(hd, read)      _(hd, write)

devops(devop_def)

static _Device x86_dev[] = {
  {_DEV_INPUT,   "8279 Keyboard Controller", input_read, NULL},
  {_DEV_TIMER,   "RDTSC Timer / CMOS RTC",   timer_read, NULL},
  {_DEV_VIDEO,   "Standard VGA Controller",  video_read, video_write},
  {_DEV_PCICONF, "PCI Configuration",        pciconf_read, pciconf_write},
  {_DEV_ATA0,    "ATA Disk Controller 0",    hd_read, hd_write},
};

int _ioe_init() {
  timer_init();
  vga_init();
  return 0;
}

_Device *_device(int n) {
  if (n >= 1 && n < sizeof(x86_dev) / sizeof(x86_dev[0])) {
    return &x86_dev[n - 1];
  } else {
    return NULL;
  }
}