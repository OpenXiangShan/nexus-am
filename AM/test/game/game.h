#include <am.h>
#include <klib.h>

const int N = 96;
const bool scale = false;
const int delay = 33; // uptime interval

extern Pixel canvas[N][N];

void redraw();
