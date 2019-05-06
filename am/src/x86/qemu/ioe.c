#include "../x86-qemu.h"

void __am_vga_init();
void __am_timer_init();

#define DEF_DEVOP(fn) \
  size_t fn(uintptr_t reg, void *buf, size_t size);

DEF_DEVOP(__am_input_read);
DEF_DEVOP(__am_timer_read);
DEF_DEVOP(__am_video_read);
DEF_DEVOP(__am_video_write);
DEF_DEVOP(__am_pciconf_read);
DEF_DEVOP(__am_pciconf_write);

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_INPUT:   return __am_input_read(reg, buf, size);
    case _DEV_TIMER:   return __am_timer_read(reg, buf, size);
    case _DEV_VIDEO:   return __am_video_read(reg, buf, size);
    case _DEV_PCICONF: return __am_pciconf_read(reg, buf, size);
  }
  return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_VIDEO:   return __am_video_write(reg, buf, size);
    case _DEV_PCICONF: return __am_pciconf_write(reg, buf, size);
  }
  return 0;
}

int _ioe_init() {
  if (_cpu() != 0) panic("init IOE in non-bootstrap CPU");
  __am_timer_init();
  __am_vga_init();
  return 0;
}
