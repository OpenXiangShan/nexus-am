#include <am.h>
#include <assert.h>
#include <SDL2/SDL.h>

#define W 640
#define H 480

_Screen _screen;

#define KEYDOWN_MASK 0x8000

static inline _Pixel pixel(u8 r, u8 g, u8 b) {
  return (r << 16) | (g << 8) | b;
}
static inline u8 R(_Pixel p) { return p >> 16; }
static inline u8 G(_Pixel p) { return p >> 8; }
static inline u8 B(_Pixel p) { return p; }

static SDL_Window *window;
static SDL_Renderer *renderer;

static int event_thread(void *args);

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN];
static int key_f = 0, key_r = 0;
static SDL_mutex *key_queue_lock;



void gui_init() {
  _screen.width = W;
  _screen.height = H;
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(W, H, 0, &window, &renderer);
  SDL_SetWindowTitle(window, "Native Application");
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  _draw_sync();
  key_queue_lock = SDL_CreateMutex();
  SDL_CreateThread(event_thread, "event thread", NULL);
}

void _draw_p(int x, int y, _Pixel p) {
  SDL_SetRenderDrawColor(renderer, R(p), G(p), B(p), 255);
  SDL_RenderDrawPoint(renderer, x, y);
}

void _draw_f(_Pixel *p) {
  for (int i = 0; i < H; i ++)
    for (int j = 0; j < W; j ++) {
      _draw_p(i, j, p[i * W + j]);
    }
}

void _draw_sync() {
  SDL_RenderPresent(renderer);
}

int _peek_key() {
  int ret = _KEY_NONE;
  SDL_LockMutex(key_queue_lock);
  if (key_f != key_r) {
    ret = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
  }
  SDL_UnlockMutex(key_queue_lock);
  return ret;
}

#define XX(k) [SDL_SCANCODE_##k] = _KEY_##k
static int keymap[256] = {
  _KEYS(XX),
};

static int event_thread(void *args) {
  SDL_Event event;
  while (1) {
    int succ = SDL_WaitEvent(&event);
    assert(succ);
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

