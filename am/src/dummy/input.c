#include <am.h>
#include <amdev.h>

size_t __am_input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;
      kbd->keydown = 0;
      kbd->keycode = _KEY_NONE;
      return sizeof(_DEV_INPUT_KBD_t);
    }
  }
  return 0;
}
