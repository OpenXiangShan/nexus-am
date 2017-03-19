#ifndef __IOM_H__
#define __IOM_H__

#include <common.h>

__H_BEGIN__

void _iom_init();

// timer
ulong _uptime();

// keyboard
enum {
  KEY_NONE = 0,
  KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
  KEY_Z, KEY_X,
};
#define KEYDOWN_MASK 0x8000 // mask bit is set for a keydown event
int _peek_key(); // peek keyboard scan code

// VGA
typedef u32 Pixel;
static inline Pixel pixel(u8 r, u8 g, u8 b) {
  return (((u32)r << 16) | ((u32)g) << 8) | b;
}
static inline u8 R(Pixel p) { return p >> 16; }
static inline u8 G(Pixel p) { return p >> 8; }
static inline u8 B(Pixel p) { return p; }

typedef struct Screen {
  int width, height; // filled by _iom_init()
} Screen;
extern Screen scr;
void _draw_p(int x, int y, Pixel p); // draw a pixel
void _draw_f(Pixel *p); // draw entire frame
void _draw_sync(); // make drawing take effect

// others should be implemented using a bus.

__H_END__

#endif
