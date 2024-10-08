#include <am.h>
#include <amdev.h>

void __am_timer_init();

int _ioe_init() {
  __am_timer_init();
  return 0;
}

size_t __am_timer_read(uintptr_t reg, void *buf, size_t size);
size_t __am_input_read(uintptr_t reg, void *buf, size_t size);

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_INPUT: return __am_input_read(reg, buf, size);
    case _DEV_TIMER: return __am_timer_read(reg, buf, size);
  }
  return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
  }
  return 0;
}
