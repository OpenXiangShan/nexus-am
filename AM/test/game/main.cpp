#include "game.h"

Pixel canvas[N][N];
bool k_up, k_down, k_left, k_right, k_z, k_x;
int X = N / 2, Y = N / 2;
int radius = 5;

const int step = 3;
static inline void dec(int &x) { if (x - step >= 0) x -= step; }
static inline void inc(int &x) { if (x + step < N) x += step; }
static inline bool valid(int x, int y) { return x >= 0 && x < N && y >= 0 && y < N; }

void update() {
  static int odd = 0;
  odd ^= 1;

  if (k_up) dec(Y);
  if (k_down) inc(Y);
  if (k_left) dec(X);
  if (k_right) inc(X);
  if (k_z) if (radius < 100) radius ++;
  if (k_x) if (radius > 2) radius --;

  for (int i = 0; i < N; i ++)
    for (int j = 0; j < N; j ++)
      canvas[i][j] = pixel(33, 33, 66);

  for (int dx = -radius; dx <= radius; dx ++)
    for (int dy = -radius; dy <= radius; dy ++) {
      if (valid(X + dx, Y + dy)) {
        if (dx * dx + dy * dy <= radius) {
          canvas[X + dx][Y + dy] = pixel(255, 0, odd * 255);
        }
      }
    }
}


int main() {
  _trm_init();
  _iom_init();

  printk("Game started.\n");

  ulong last = 0;

  while (true) {
    ulong upt = _uptime();
    while (true) {
      int ev = _peek_key();
      int k = ev & ~KEYDOWN_MASK;
      int down = (ev & KEYDOWN_MASK) != 0;
      if (k == KEY_NONE) break;
      switch (k) {
        case KEY_UP: k_up = down; break;
        case KEY_DOWN: k_down = down; break;
        case KEY_LEFT: k_left = down; break;
        case KEY_RIGHT: k_right = down; break;
        case KEY_Z: k_z = down; break;
        case KEY_X: k_x = down; break;
        default: break;
      }
    }
    if (upt - last > delay) {
      update();
      redraw();
      last = upt;
    }
  }

  return 0;
}
