#include <am.h>
#include <amdev.h>

void __am_vga_init();
void __am_timer_init();
void __am_audio_init();

int _ioe_init() {
  __am_vga_init();
  __am_timer_init();
  __am_audio_init();
  return 0;
}

size_t __am_timer_read(uintptr_t reg, void *buf, size_t size);
size_t __am_video_read(uintptr_t reg, void *buf, size_t size);
size_t __am_video_write(uintptr_t reg, void *buf, size_t size);
size_t __am_input_read(uintptr_t reg, void *buf, size_t size);
size_t __am_audio_read(uintptr_t reg, void *buf, size_t size);
size_t __am_audio_write(uintptr_t reg, void *buf, size_t size);

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_INPUT: return __am_input_read(reg, buf, size);
    case _DEV_TIMER: return __am_timer_read(reg, buf, size);
    case _DEV_VIDEO: return __am_video_read(reg, buf, size);
    case _DEV_AUDIO: return __am_audio_read(reg, buf, size);
  }
  return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  switch (dev) {
    case _DEV_VIDEO: return __am_video_write(reg, buf, size);
    case _DEV_AUDIO: return __am_audio_write(reg, buf, size);
  }
  return 0;
}
