#include <am.h>
#include <amdev.h>

void timer_init();
void video_init();

uintptr_t input_read(uintptr_t, size_t);
uintptr_t timer_read(uintptr_t, size_t);
uintptr_t video_read(uintptr_t, size_t);

void video_write(uintptr_t, size_t, uintptr_t);

static _Device devices[] = {
  {_DEV_INPUT,   "SDL Keyboard", input_read},
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
