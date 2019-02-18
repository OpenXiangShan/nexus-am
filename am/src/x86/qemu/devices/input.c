#include "../../x86-qemu.h"

static int scan_code[] = {
   0, 1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88,
     41,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 43,
     58, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 28,
     42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
     29, 91, 56, 57, 56, 29, 
     72, 80, 75, 77, 0, 0, 0, 0, 0, 0
};

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;

  int status = inb(0x64);
  kbd->keydown = 0;
  kbd->keycode = _KEY_NONE;

  if ((status & 0x1) == 0) {
  } else {
    if (status & 0x20) { // mouse
    } else {
      int code = inb(0x60) & 0xff;

      for (int i = 0; i < sizeof(scan_code) / sizeof(scan_code[0]); i ++) {
        if (scan_code[i] == 0) continue;
        if (scan_code[i] == code) {
          kbd->keydown = 1;
          kbd->keycode = i;
          break;
        } else if (scan_code[i] + 128 == code) {
          kbd->keydown = 0;
          kbd->keycode = i;
          break;
        }
      }
    }
  }
  return sizeof(*kbd);
}
