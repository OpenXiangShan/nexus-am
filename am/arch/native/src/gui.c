#include <am.h>

_Screen _screen;

// TODO: this condition should be changed
#if !defined(__i386__) && !defined(__x86_64__)

// without GUI

void gui_init() { _screen.width = _screen.height = 0; }
void _draw_p(int x, int y, _Pixel p) {}
void _draw_f(_Pixel *p) {}
void _draw_sync() {}
int _peek_key() { return _KEY_NONE; }

#else

// with GUI (allegro5)

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define W 640
#define H 480

#define KEYDOWN_MASK 0x8000

static inline _Pixel pixel(u8 r, u8 g, u8 b) {
  return (r << 16) | (g << 8) | b;
}
static inline u8 R(_Pixel p) { return p >> 16; }
static inline u8 G(_Pixel p) { return p >> 8; }
static inline u8 B(_Pixel p) { return p; }

static ALLEGRO_VERTEX vtx[W * H];

void gui_init() {
  _screen.width = W;
  _screen.height = H;
  al_init();
  al_init_primitives_addon();
  al_install_keyboard();
  al_create_display(_screen.width, _screen.height);
  
  for (int x = 0; x < W; x ++)
    for (int y = 0; y < H; y ++) {
      int i = x + y * _screen.width;
      vtx[i].x = x;
      vtx[i].y = y;
    }
}

void _draw_p(int x, int y, _Pixel p) {
  vtx[x + y * _screen.width].color = al_map_rgb(R(p), G(p), B(p));
}

void _draw_f(_Pixel *p) {
  int sz = _screen.width * _screen.height;
  for (int i = 0; i < sz; i ++) {
    vtx[i].color = al_map_rgb(R(p[i]), G(p[i]), B(p[i]));
  }
}

void _draw_sync() {
    al_draw_prim(vtx, NULL, NULL, 0, _screen.width * _screen.height, ALLEGRO_PRIM_POINT_LIST);
    al_flip_display();
}

static int k_up, k_down, k_left, k_right, k_z, k_x;

static void update(int k, int *ev, int *b, int down) {
  int evk = *ev & ~KEYDOWN_MASK;
  if (evk != _KEY_NONE) return;
  if (*b == down) {
    (*ev) = _KEY_NONE;
  } else {
    (*b) = down;
    (*ev) = k | down * KEYDOWN_MASK;
  }
}

int _peek_key() {
  ALLEGRO_KEYBOARD_STATE state;
  al_get_keyboard_state(&state);

  int ev = _KEY_NONE;

  update(_KEY_UP, &ev, &k_up, al_key_down(&state, ALLEGRO_KEY_UP));
  update(_KEY_DOWN, &ev, &k_down, al_key_down(&state, ALLEGRO_KEY_DOWN));
  update(_KEY_LEFT, &ev, &k_left, al_key_down(&state, ALLEGRO_KEY_LEFT));
  update(_KEY_RIGHT, &ev, &k_right, al_key_down(&state, ALLEGRO_KEY_RIGHT));
  update(_KEY_Z, &ev, &k_z, al_key_down(&state, ALLEGRO_KEY_Z));
  update(_KEY_X, &ev, &k_x, al_key_down(&state, ALLEGRO_KEY_X));

  return ev;
}

#endif
