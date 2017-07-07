#include "common.h"

size_t read_events(void *buf) {
  uint32_t time_ms = _uptime();
  int key = _read_key();
  char keydown_char = (key & 0x8000 ? 'd' : 'u');
  char *keyname = "NO_KEY";
  key &= ~0x8000;
  if (key == _KEY_SPACE) {
    keyname = "SPACE";
  }

  sprintf(buf, "t %d\nk%c %s ", time_ms, keydown_char, keyname);
  int ret = strlen(buf);
  return ret;
}
