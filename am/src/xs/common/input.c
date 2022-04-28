#include <am.h>
#include <amdev.h>
#include <nemu.h>
#include <klib.h>

#define KEYDOWN_MASK 0x8000

int __am_uartlite_getchar();

static int am_keycode[128] = {
  [' '] = _KEY_SPACE,
  ['1'] = _KEY_1, ['2'] = _KEY_2, ['3'] = _KEY_3, ['4'] = _KEY_4,
  ['5'] = _KEY_5, ['6'] = _KEY_6, ['7'] = _KEY_7, ['8'] = _KEY_8,
  ['9'] = _KEY_9, ['0'] = _KEY_0,
  ['a'] = _KEY_A, ['b'] = _KEY_B, ['c'] = _KEY_C, ['d'] = _KEY_D,
  ['e'] = _KEY_E, ['f'] = _KEY_F, ['g'] = _KEY_G, ['h'] = _KEY_H,
  ['i'] = _KEY_I, ['j'] = _KEY_J, ['k'] = _KEY_K, ['l'] = _KEY_L,
  ['m'] = _KEY_M, ['n'] = _KEY_N, ['o'] = _KEY_O, ['p'] = _KEY_P,
  ['q'] = _KEY_Q, ['r'] = _KEY_R, ['s'] = _KEY_S, ['t'] = _KEY_T,
  ['u'] = _KEY_U, ['v'] = _KEY_V, ['w'] = _KEY_W, ['x'] = _KEY_X,
  ['y'] = _KEY_Y, ['z'] = _KEY_Z
};

size_t __am_input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;
      static int last_key = _KEY_NONE;
      static int last_release = 0;
      static unsigned long timestamp = 0;

      kbd->keydown = 0;
      kbd->keycode = _KEY_NONE;

      if (last_release) {
        // and return the makecode of the last key
        kbd->keydown = 1;
        kbd->keycode = last_key;
        last_release = 0;
        timestamp = uptime();
      } else {
        int _k = __am_uartlite_getchar();
        if (_k == 255) _k = 0;
        assert(_k < 128 && _k >= 0);
        int key = (_k == -1 ? _KEY_NONE : am_keycode[_k]);
        if (key == _KEY_NONE) {
          if (last_key != _KEY_NONE) {
            unsigned long now = uptime();
            if (now - timestamp > 500) {
              kbd->keydown = 0;
              kbd->keycode = last_key;
              last_key = _KEY_NONE;
            }
          }
        } else {
          int another_key = (key != _KEY_NONE && last_key != _KEY_NONE && key != last_key);
          // if we press a different key, now return the breakcode of the old key,
          // and return the makecode of new key next time
          kbd->keydown = another_key ? 0 : 1;
          kbd->keycode = another_key ? last_key : key;
          last_key = key;
          last_release = another_key;
          if (!another_key) timestamp = uptime();
        }
      }

      return sizeof(_DEV_INPUT_KBD_t);
    }
  }
  return 0;
}
