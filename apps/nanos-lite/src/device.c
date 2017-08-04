#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

const char *names[256] = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

static unsigned long last_time_ms = 0;

size_t read_events(void *buf) {
  int key = _read_key();
  char keydown_char = (key & 0x8000 ? 'd' : 'u');
  key &= ~0x8000;
  if (key != _KEY_NONE) {
    return sprintf(buf, "k%c %s\n", keydown_char, names[key]) - 1;
  }
  else {
    unsigned long time_ms;
    while ( (time_ms = _uptime()) - last_time_ms < 33 );
    last_time_ms = time_ms;
    return sprintf(buf, "t %d\n", time_ms) - 1;
  }
}

static char dispinfo[128];

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  offset /= sizeof(uint32_t);
  _draw_rect(buf, offset % _screen.width, offset / _screen.width, len / sizeof(uint32_t), 1);
}

void init_device() {
  _ioe_init();
  sprintf(dispinfo, "WIDTH: %d\nHEIGHT: %d", _screen.width, _screen.height);
}
