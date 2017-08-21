#ifndef __DEV_H__
#define __DEV_H__


// Device list

#define _DEV_AM_PERFCNT   0x0000ac01
#define _DEV_AM_CONSOLE   0x0000ac00
#define _DEV_AM_TIMER     0x0000ac00
#define _DEV_AM_VIDEO     0x0000ac00

#define _DEV_PCI_CONF     0x00000080
#define _DEV_ATA0         0x00000dd0
#define _DEV_ATA1         0x00000dd1



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

#endif
