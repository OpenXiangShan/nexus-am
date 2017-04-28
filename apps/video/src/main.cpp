#include <am.h>
#include <video.h>
#include <klib.h>

_Pixel canvas[N][N];
bool used[N][N];

void redraw() {
  int w = _screen.width;
  int h = _screen.height;

  for (int x = 0; x < w; x ++) {
    for (int y = 0; y < h; y ++) {
      _draw_p(x, y, canvas[x * N / w][y * N / h]);
    }
  }
   _draw_sync();
}

static _Pixel p(int tsc) {
  int b = tsc & 0xff;
  return pixel(b * 6, b * 7, b);
}

void update() {
  static int tsc = 0;
  static int dx[4] = {0, 1, 0, -1};
  static int dy[4] = {1, 0, -1, 0};

  tsc ++;

  for (int i = 0; i < N; i ++)
    for (int j = 0; j < N; j ++) {
      used[i][j] = false;
    }

  int init = tsc * 1;
  canvas[0][0] = p(init); used[0][0] = true;
  int x = 0, y = 0, d = 0;
  for (int step = 1; step < N * N; step ++) {
    for (int t = 0; t < 4; t ++) {
      int x1 = x + dx[d], y1 = y + dy[d];
      if (x1 >= 0 && x1 < N && y1 >= 0 && y1 < N && !used[x1][y1]) {
        x = x1; y = y1;
        used[x][y] = true;
        canvas[x][y] = p(init + step / 2);
        break;
      }
      d = (d + 1) % 4;
    }
  }
}


int main() {
  _trm_init();
  _ioe_init();
  _asye_init();

  ulong last = 0;

  while (true) {
    ulong upt = _uptime();
    if (upt - last > 1000 / FPS) {
      update();
      redraw();
      last = upt;
    }
  }

  return 0;
}
