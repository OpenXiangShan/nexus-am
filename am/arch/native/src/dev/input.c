#include <am.h>
#include <amdev.h>
#include <SDL2/SDL.h>

#define KEYDOWN_MASK 0x8000

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN];
static int key_f = 0, key_r = 0;
static SDL_mutex *key_queue_lock;

#define XX(k) [SDL_SCANCODE_##k] = _KEY_##k,
static int keymap[256] = {
  _KEYS(XX)
};

static int event_thread(void *args) {
  SDL_Event event;
  while (1) {
    SDL_WaitEvent(&event);
    switch (event.type) {
      case SDL_QUIT: exit(0); break;
      case SDL_KEYDOWN: 
      case SDL_KEYUP:
        {
          SDL_Keysym k = event.key.keysym;
          int keydown = event.key.type == SDL_KEYDOWN;
          if (event.key.repeat == 0) {
            int scancode = k.scancode;
            if (keymap[scancode] != 0) {
              int am_code = keymap[scancode] | (keydown ? KEYDOWN_MASK : 0);
              SDL_LockMutex(key_queue_lock);
              key_queue[key_r] = am_code;
              key_r = (key_r + 1) % KEY_QUEUE_LEN;
              SDL_UnlockMutex(key_queue_lock);
            }
          }
        }
        break;
    }
  }
}

void input_init() {
  key_queue_lock = SDL_CreateMutex();
  SDL_CreateThread(event_thread, "event thread", NULL);
}


static int scan_code[] = {
  0,
  1, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 87, 88,
  41, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 43,
  58, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 28,
  42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
  29, 91, 56, 57, 56, 29, 
  72, 80, 75, 77, 0, 0, 0, 0, 0, 0
};

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  _KbdReg *kbd = (_KbdReg *)buf;

  int status = 0; // inb(0x64);
  kbd->keydown = 0;
  kbd->keycode = _KEY_NONE;

  if ((status & 0x1) == 0) {
  } else {
    if (status & 0x20) { // mouse
    } else {
      // int code = inb(0x60) & 0xff;
      int code = 0x59;

      for (unsigned int i = 0; i < sizeof(scan_code) / sizeof(int); i ++) {
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

  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _KbdReg *kbd = (_KbdReg *)buf;
      int k = _KEY_NONE;

      SDL_LockMutex(key_queue_lock);
      if (key_f != key_r) {
        k = key_queue[key_f];
        key_f = (key_f + 1) % KEY_QUEUE_LEN;
      }
      SDL_UnlockMutex(key_queue_lock);

      kbd->keydown = (k & KEYDOWN_MASK ? 1 : 0);
      kbd->keycode = k & ~KEYDOWN_MASK;

      return sizeof(_KbdReg);
    }
  }
  return 0;
}
