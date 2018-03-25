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
  _UptimeReg uptime;
  _Device *dev = getdev(&timer_dev, _DEV_TIMER);
  dev->read(_DEVREG_TIMER_UPTIME, &uptime, sizeof(uptime));
  return uptime.lo;
}

void get_timeofday(void *rtc) {
  _Device *dev = getdev(&timer_dev, _DEV_TIMER);
  dev->read(_DEVREG_TIMER_DATE, rtc, sizeof(_RTCReg));
}

int read_key() {
  _Device *dev = getdev(&input_dev, _DEV_INPUT);
  _KbdReg key;
  dev->read(_DEVREG_INPUT_KBD, &key, sizeof(_KbdReg));
  int ret = key.keycode;
  if (key.keydown) ret |= 0x8000;
  return ret;
}

void draw_rect(uint32_t *pixels, int x, int y, int w, int h) {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _FBCtlReg ctl;
  ctl.pixels = pixels;
  ctl.x = x;
  ctl.y = y;
  ctl.w = w;
  ctl.h = h;
  ctl.sync = 0;
  dev->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
}

void draw_sync() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _FBCtlReg ctl;
  ctl.pixels = NULL;
  ctl.x = ctl.y = ctl.w = ctl.h = 0;
  ctl.sync = 1;
  dev->write(_DEVREG_VIDEO_FBCTL, &ctl, sizeof(ctl));
}

int screen_width() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _VideoInfoReg info;
  dev->read(_DEVREG_VIDEO_INFO, &info, sizeof(info));
  return info.width;
}

int screen_height() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _VideoInfoReg info;
  dev->read(_DEVREG_VIDEO_INFO, &info, sizeof(info));
  return info.height;
}

