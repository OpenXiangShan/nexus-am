#include <am.h>
#include <amdev.h>

void timer_init();
void video_init();
void input_init();

size_t input_read(uintptr_t reg, void *buf, size_t size);
size_t timer_read(uintptr_t reg, void *buf, size_t size);
size_t video_read(uintptr_t reg, void *buf, size_t size);
size_t video_write(uintptr_t reg, void *buf, size_t size);

int _ioe_init() {
  timer_init();
  video_init();
  input_init();
  return 0;
}

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_INPUT: return input_read(reg, buf, size);
    case _DEV_TIMER: return timer_read(reg, buf, size);
    case _DEV_VIDEO: return video_read(reg, buf, size);
  }
  return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_VIDEO: return video_write(reg, buf, size);
  }
  return 0;
}

