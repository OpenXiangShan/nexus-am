#include <am.h>
#include <amdev.h>
#include <ndl.h>

#define KEYDOWN_MASK 0x8000

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0;

#define XX(k) [NDL_SCANCODE_##k] = _KEY_##k,
static int keymap[256] = {
  _KEYS(XX)
};

void __am_set_systime(uint32_t t);

int __am_event_thread(void) {
  NDL_Event event;
  NDL_WaitEvent(&event);

  if (event.type == NDL_EVENT_TIMER) {
    __am_set_systime(event.data);
  }
  
  if (event.type == NDL_EVENT_KEYUP || event.type == NDL_EVENT_KEYDOWN) {
    int keydown = event.type == NDL_EVENT_KEYDOWN;
    int scancode = event.data;
    if (keymap[scancode] != 0) {
      int am_code = keymap[scancode] | (keydown ? KEYDOWN_MASK : 0);
      key_queue[key_r] = am_code;
      key_r = (key_r + 1) % KEY_QUEUE_LEN;
    }
    return true;
  }

  return false;
}

size_t __am_input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      while (__am_event_thread());
      _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;
      int k = _KEY_NONE;

      if (key_f != key_r) {
        k = key_queue[key_f];
        key_f = (key_f + 1) % KEY_QUEUE_LEN;
      }

      kbd->keydown = (k & KEYDOWN_MASK ? 1 : 0);
      kbd->keycode = k & ~KEYDOWN_MASK;

      return sizeof(_DEV_INPUT_KBD_t);
    }
  }
  return 0;
}
