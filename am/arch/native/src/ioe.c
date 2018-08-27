#include <am.h>
#include <amdev.h>

void timer_init();
void video_init();
void input_init();

size_t input_read(uintptr_t reg, void *buf, size_t size);
size_t timer_read(uintptr_t reg, void *buf, size_t size);
size_t video_read(uintptr_t reg, void *buf, size_t size);

size_t video_write(uintptr_t reg, void *buf, size_t size);

static _Device devices[] = {
  {_DEV_INPUT,   "SDL Keyboard", input_read, NULL},
  {_DEV_TIMER,   "Native Timer", timer_read, NULL},
  {_DEV_VIDEO,   "SDL Graphics", video_read, video_write},
};

int _ioe_init() {
  timer_init();
  video_init();
  input_init();
  return 0;
}

_Device *_device(int n) {
  if (n >= 1 && n <= sizeof(devices) / sizeof(devices[0])) {
    return &devices[n - 1];
  } else {
    return NULL;
  }
}
