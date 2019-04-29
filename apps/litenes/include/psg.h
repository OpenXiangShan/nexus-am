// #define psg_io_read(...) 0xFF
// #define psg_io_write(...) /**/

#include "common.h"

#ifndef PSG_H
#define PSG_H

extern int key_p;

static inline byte psg_io_read(word address) {
  static int const MAP[256] = {
    0, // On/Off
    _KEY_G, // A
    _KEY_H, // B
    _KEY_T, // SELECT
    _KEY_Y, // START
    _KEY_W,
    _KEY_S,
    _KEY_A,
    _KEY_D,
    255,
  };
  extern int key_state[];

  // Joystick 1
  if (address == 0x4016) {
    if (key_p++ < 9) {
      return key_state[MAP[key_p]];
    }
  }
  return 0;
}

static inline void psg_io_write(word address, byte data) {
  static byte prev_write;
  if (address == 0x4016) {
    if ((data & 1) == 0 && prev_write == 1) {
      // strobe
      key_p = 0;
    }
  }
  prev_write = data & 1;
}

#endif
