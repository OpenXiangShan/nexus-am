#ifndef __AMDEV_H__
#define __AMDEV_H__

#include <stdint.h>

#define _DEV_PERFCNT 0x0000ac01
#define _DEV_INPUT   0x0000ac02
#define _DEV_TIMER   0x0000ac03
#define _DEV_VIDEO   0x0000ac04
#define _DEV_PCICONF 0x00000080
#define _DEV_ATA0    0x00000dd0
#define _DEV_ATA1    0x00000dd1

// Performance Counter (0000ac01)

// Input (0000ac02)

#define _DEV_INPUT_REG_KBD   1
#define _KEYS(_) \
  _(ESCAPE) \
  _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) \
  _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) \
  _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) \
  _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) \
  _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) \
  _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)
#define _KEY_NAME(k) _KEY_##k,
enum {
  _KEY_NONE = 0,
  _KEYS(_KEY_NAME)
};

// Timer (0000ac03)

typedef struct _TimerUptime {
  uint32_t hi, lo;
} _Dev_Timer_Uptime;
#define _DEV_TIMER_REG_UPTIME  1
typedef struct _TimerRTC {
  int year, month, day, hour, minute, second;
} _Dev_Timer_RTC;
#define _DEV_TIMER_REG_DATE    2

// Video (0000ac04)

typedef struct _VideoInfo {
  int32_t width, height;
} _Dev_Video_Info;
#define _DEV_VIDEO_REG_INFO    1
typedef struct _Video_FBCtl {
  int x, y, w, h, sync;
  uint32_t *pixels;
} _Dev_Video_FBCtl;
#define _DEV_VIDEO_REG_FBCTL   2

// PCI Configuration (00000080)

static inline uint32_t
_DEV_PCICONF_REG(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
  return ((uint32_t)1 << 31)
       | ((uint32_t)bus << 16)
       | ((uint32_t)slot << 11)
       | ((uint32_t)func << 8)
       | offset;
}

// ATA (00000dd0 -- 00000dd1)

#define _DEV_ATA_REG_DATA      0
#define _DEV_ATA_REG_FEATURE   1
#define _DEV_ATA_REG_NSECT     2
#define _DEV_ATA_REG_SECT      3
#define _DEV_ATA_REG_CYLOW     4
#define _DEV_ATA_REG_CYHIGH    5
#define _DEV_ATA_REG_DRIVE     6
#define _DEV_ATA_REG_STATUS    7

#endif
