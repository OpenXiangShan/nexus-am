#include "../x86-qemu.h"

void vga_init();
void timer_init();

#define DEF_DEVOP(fn) \
  size_t fn(uintptr_t reg, void *buf, size_t size);

DEF_DEVOP(input_read);
DEF_DEVOP(timer_read);
DEF_DEVOP(video_read);
DEF_DEVOP(video_write);
DEF_DEVOP(pciconf_read);
DEF_DEVOP(pciconf_write);

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_INPUT: return input_read(reg, buf, size);
    case _DEV_TIMER: return timer_read(reg, buf, size);
    case _DEV_VIDEO: return video_read(reg, buf, size);
    case _DEV_PCICONF: return pciconf_read(reg, buf, size);
  }
  return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_VIDEO: return video_write(reg, buf, size);
    case _DEV_PCICONF: return pciconf_write(reg, buf, size);
  }
  return 0;
}

int _ioe_init() {
  if (_cpu() != 0) panic("init IOE in non-bootstrap CPU");
  timer_init();
  vga_init();
  return 0;
}