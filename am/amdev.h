#ifndef __DEV_H__
#define __DEV_H__

#define _DEV_PERFCNT 0x0000ac01
#define _DEV_INPUT   0x0000ac02
#define _DEV_TIMER   0x0000ac03
#define _DEV_VIDEO   0x0000ac04
#define _DEV_PCICONF 0x00000080
#define _DEV_ATA0    0x00000dd0
#define _DEV_ATA1    0x00000dd1


// performance counter (0000ac01)

// console (0000ac02)

#define _DEV_INPUT_REG_KBD   1

#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define _KEY_NAME(k) _KEY_##k,

enum {
  _KEY_NONE = 0,
  _KEYS(_KEY_NAME)
};

// timer (0000ac01)

#define _DEV_TIMER_REG_UPTIME  1

// video (0000ac01)

#define _DEV_VIDEO_REG_WIDTH   1
#define _DEV_VIDEO_REG_HEIGHT  2
#define _DEV_VIDEO_REG_PIXELS  3
#define _DEV_VIDEO_REG_X       4
#define _DEV_VIDEO_REG_Y       5
#define _DEV_VIDEO_REG_W       6
#define _DEV_VIDEO_REG_H       7
#define _DEV_VIDEO_REG_DRAW    8
#define _DEV_VIDEO_REG_SYNC    9

#endif
