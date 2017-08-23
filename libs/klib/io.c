#include <klib.h>
#include <amdev.h>

static _Device *getdev(_Device **ptr, uint32_t id) {
  if (*ptr) return *ptr;
  for (int n = 1; ; n ++) {
    _Device *cur = _device(n);
    if (cur->id == id) {
      *ptr = cur;
      return cur;
    }
    if (!cur) break;
  }
  assert(0);
  return NULL;
}

static _Device *input_dev;
static _Device *video_dev;
static _Device *timer_dev;

uint32_t uptime() {
  _Device *dev = getdev(&timer_dev, _DEV_TIMER);
  return dev->read(_DEV_TIMER_REG_UPTIME, 4);
}

int read_key() {
  _Device *dev = getdev(&input_dev, _DEV_INPUT);
  return dev->read(_DEV_INPUT_REG_KBD, 2);
}

void draw_rect(uint32_t *pixels, int x, int y, int w, int h) {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  dev->write(_DEV_VIDEO_REG_PIXELS, 4, (uintptr_t)pixels);
  dev->write(_DEV_VIDEO_REG_X, 4, x);
  dev->write(_DEV_VIDEO_REG_Y, 4, y);
  dev->write(_DEV_VIDEO_REG_W, 4, w);
  dev->write(_DEV_VIDEO_REG_H, 4, h);
  dev->write(_DEV_VIDEO_REG_DRAW, 4, 0);
}

void draw_sync() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  dev->write(_DEV_VIDEO_REG_SYNC, 4, 0);
}

int screen_width() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  return dev->read(_DEV_VIDEO_REG_WIDTH, 4);
}

int screen_height() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  return dev->read(_DEV_VIDEO_REG_HEIGHT, 4);
}

