#include <am.h>
#include <amdev.h>
#include <klib.h>

static inline size_t no_read(uintptr_t reg, void *buf, size_t size) {
  assert(0);
  return 0;
}

static inline size_t no_write(uintptr_t reg, void *buf, size_t size) {
  assert(0);
  return 0;
}

void vga_init();
void timer_init();

int _ioe_init() {
  vga_init();
  timer_init();
  return 0;
}

size_t timer_read(uintptr_t reg, void *buf, size_t size);
size_t video_read(uintptr_t reg, void *buf, size_t size);
size_t video_write(uintptr_t reg, void *buf, size_t size);
size_t input_read(uintptr_t reg, void *buf, size_t size);


static _Device n86_dev[] = {
  {_DEV_TIMER,   "NEMU Timer", timer_read, no_write},
  {_DEV_INPUT,   "NEMU Keyboard Controller", input_read, no_write},
  {_DEV_VIDEO,   "NEMU VGA Controller", video_read, video_write},
};

#define NR_DEV (sizeof(n86_dev) / sizeof(n86_dev[0]))

_Device *_device(int n) {
  n --;
  return (n >= 0 && (unsigned int)n < NR_DEV) ? &n86_dev[n] : NULL;
}
