#include "game.h"

void redraw() {
  static Pixel fb[640 * 480];
  int w = scr.width;
  int h = scr.height;
  assert(w >= h);

  int pad = (w - h) / 2;
  if (scale) {
    for (int x = 0; x < w; x ++) {
      for (int y = 0; y < h; y ++) {
        fb[x + w * y] = canvas[x * N / w][y * N / h];
      }
    }
  } else {
    for (int x = pad; x < w - pad; x ++) {
      for (int y = 0; y < h; y ++) {
        fb[x + w * y] = canvas[(x - pad) * N / h][y * N / h];
      }
    }
  }
  _draw_f(fb);
  _draw_sync();
}

