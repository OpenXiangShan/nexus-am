#include "psg.h"
#include "hal.h"

static byte prev_write;
static int p = 10;

static int MAP[256] = {
  0, // On/Off
  _KEY_Z, // A
  _KEY_X, // B
  _KEY_UP, // SELECT
  _KEY_DOWN, // START
  0,
  0,
  _KEY_LEFT, 
  _KEY_RIGHT, 
};

extern int key_state[];

#include <stdio.h>

inline byte psg_io_read(word address)
{
    // Joystick 1
    if (address == 0x4016) {
        if (p++ < 9) {
          if (key_state[MAP[p]]) {
            printf("Detected key press: %d\n", p);
          }
          return key_state[MAP[p]];
        }
    }
    return 0;
}

inline void psg_io_write(word address, byte data)
{
    if (address == 0x4016) {
        if ((data & 1) == 0 && prev_write == 1) {
            // strobe
            p = 0;
        }
    }
    prev_write = data & 1;
}
