#ifndef FCE_H
#define FCE_H

#include <am.h>
#include <common.h>

//#define STRETCH

#define FPS 60
#define W   256
#define H   240

int fce_load_rom(char *rom);
void fce_init();
void fce_run();
void fce_update_screen();

extern char rom_mario_nes[];

static inline void draw(int col, int row, int idx) {
#ifdef STRETCH
  extern byte canvas[257][520];
  // prevent out-of-bound while mario jump above the sky
  if (row < 257) {
    canvas[row][col + 0xff] = idx;
  }
#else
  extern const uint32_t palette[64];
  extern uint32_t screen[H][W + 16];
  screen[row][col] = palette[idx];
#endif
}

static inline void draw_color(int col, int row, uint32_t c) {
#ifdef STRETCH
  // not support stretch mode yet
  assert(0);
#else
  extern uint32_t screen[H][W + 16];
  screen[row][col] = c;
#endif
}

#endif
