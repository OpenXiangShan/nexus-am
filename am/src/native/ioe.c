#include <am.h>
#include <amdev.h>

void __am_timer_init();
void __am_video_init();
void __am_input_init();

size_t __am_input_read(uintptr_t reg, void *buf, size_t size);
size_t __am_timer_read(uintptr_t reg, void *buf, size_t size);
size_t __am_video_read(uintptr_t reg, void *buf, size_t size);
size_t __am_video_write(uintptr_t reg, void *buf, size_t size);

static int init_flag = 0;

int _ioe_init() {
  if (_cpu() != 0) return 0;
  init_flag ++;
  if (init_flag == 1) {
    // Calling fork() in MPE after SDL_Init() will cause trouble.
    // Postpone the ioe initialization to fix this issue.
    return 0;
  }

  __am_timer_init();
  __am_video_init();
  __am_input_init();
  return 0;
}

size_t _io_read(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  if (init_flag == 1) { _ioe_init(); }
  switch (dev) {
    case _DEV_INPUT: return __am_input_read(reg, buf, size);
    case _DEV_TIMER: return __am_timer_read(reg, buf, size);
    case _DEV_VIDEO: return __am_video_read(reg, buf, size);
  }
  return 0;
}

size_t _io_write(uint32_t dev, uintptr_t reg, void *buf, size_t size) {
  if (init_flag == 1) { _ioe_init(); }
  switch (dev) {
    case _DEV_VIDEO: return __am_video_write(reg, buf, size);
  }
  return 0;
}
