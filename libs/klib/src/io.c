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
  _Dev_Timer_Uptime uptime;
  _Device *dev = getdev(&timer_dev, _DEV_TIMER);
  dev->read(_DEV_TIMER_REG_UPTIME, &uptime, sizeof(uptime));
  return uptime.lo;
}

void gettimeofday(_Dev_Timer_RTC *rtc) {
  _Device *dev = getdev(&timer_dev, _DEV_TIMER);
  dev->read(_DEV_TIMER_REG_DATE, rtc, sizeof(_DEV_TIMER_REG_DATE));
}

int read_key() {
  _Device *dev = getdev(&input_dev, _DEV_INPUT);
  int32_t key;
  dev->read(_DEV_INPUT_REG_KBD, &key, sizeof(int32_t));
  return key;
}

void draw_rect(uint32_t *pixels, int x, int y, int w, int h) {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _Dev_Video_FBCtl ctl;
  ctl.pixels = pixels;
  ctl.x = x;
  ctl.y = y;
  ctl.w = w;
  ctl.h = h;
  ctl.sync = 0;
  dev->write(_DEV_VIDEO_REG_FBCTL, &ctl, sizeof(ctl));
}

void draw_sync() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _Dev_Video_FBCtl ctl;
  ctl.pixels = NULL;
  ctl.x = ctl.y = ctl.w = ctl.h = 0;
  ctl.sync = 1;
  dev->write(_DEV_VIDEO_REG_FBCTL, &ctl, sizeof(ctl));
}

int screen_width() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _Dev_Video_Info info;
  dev->read(_DEV_VIDEO_REG_INFO, &info, sizeof(info));
  return info.width;
}

int screen_height() {
  _Device *dev = getdev(&video_dev, _DEV_VIDEO);
  _Dev_Video_Info info;
  dev->read(_DEV_VIDEO_REG_INFO, &info, sizeof(info));
  return info.height;
}

