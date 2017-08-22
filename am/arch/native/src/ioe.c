#include <am.h>
#include <amdev.h>

void timer_init();
void video_init();

uintptr_t console_read(uintptr_t, size_t);
uintptr_t timer_read(uintptr_t, size_t);
uintptr_t video_read(uintptr_t, size_t);

void console_write(uintptr_t, size_t, uintptr_t);
void video_write(uintptr_t, size_t, uintptr_t);

static _Device devices[] = {
  {_DEV_CONSOLE, "Native Console", console_read, console_write},
  {_DEV_TIMER,   "Native Timer", timer_read, NULL},
  {_DEV_VIDEO,   "SDL Graphics", video_read, video_write},
};

void _ioe_init() {
  timer_init();
  video_init();
}

_Device *_device(int n) {
  n --;
  if (n >= 0 && n < sizeof(devices) / sizeof(devices[0])) {
    return &devices[n];
  } else {
    return NULL;
  }
}
