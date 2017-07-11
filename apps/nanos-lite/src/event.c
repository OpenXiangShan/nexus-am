#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

const char *names[256] = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t read_events(void *buf) {
  uint32_t time_ms = _uptime();
  int key = _read_key();
  char keydown_char = (key & 0x8000 ? 'd' : 'u');
  key &= ~0x8000;

  return sprintf(buf, "t %d\nk%c %s ", time_ms, keydown_char, names[key]);
}
