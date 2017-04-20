#include <am.h>

const int FPS = 30;
const int N = 96;
const bool scale = true;

static inline _Pixel pixel(u8 r, u8 g, u8 b) {
  return (r << 16) | (g << 8) | b;
}
static inline u8 R(_Pixel p) { return p >> 16; }
static inline u8 G(_Pixel p) { return p >> 8; }
static inline u8 B(_Pixel p) { return p; }

extern _Pixel canvas[N][N];

void redraw();
