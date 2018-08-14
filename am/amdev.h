#ifndef __AMDEV_H__
#define __AMDEV_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =========================== AM Devices ============================
// ((_Device *)dev)->id

#define _DEV_PERFCNT 0x0000ac01 // AM Virtual Performance Counter 
#define _DEV_INPUT   0x0000ac02 // AM Virtual Input Device
#define _DEV_TIMER   0x0000ac03 // AM Virtual Timer
#define _DEV_VIDEO   0x0000ac04 // AM Virtual Video Controller
#define _DEV_SERIAL  0x0000ac05 // AM Virtual Serial
#define _DEV_PCICONF 0x00000080 // PCI Configuration Space
#define _DEV_ATA0    0x00000dd0 // Primary ATA
#define _DEV_ATA1    0x00000dd1 // Secondary ATA

// ================= Device Register Specifications ==================

// --------- _DEV_PERFCNT AM Performance Counter (0000ac01) ----------

// ------------- _DEV_INPUT: AM Input Devices (0000ac02) -------------
#define _DEVREG_INPUT_KBD     1
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
  typedef struct {
    int keydown; // is keydown ? 1 : 0
    int keycode; // key code _KEY_XXX
  } _KbdReg;

// ----------- _DEV_TIMER: AM Real Time Clock (0000ac03) -------------
#define _DEVREG_TIMER_UPTIME  1
  typedef struct {
    uint32_t hi; // high 32bit of uptime (ms)
    uint32_t lo; //  low 32bit of uptime (ms)
  } _UptimeReg;

#define _DEVREG_TIMER_DATE    2
  typedef struct {
    int year, month, day;     // date
    int hour, minute, second; // time
  } _RTCReg;

// ----------- _DEV_VIDEO: AM Video Controller (0000ac04) ------------
#define _DEVREG_VIDEO_INFO    1
  typedef struct {
    int32_t width, height; // screen size: @width * @height
  } _VideoInfoReg;

#define _DEVREG_VIDEO_FBCTL   2
  typedef struct {
    int x, y;         // draw to (@x, @y)
    uint32_t *pixels; // @pixels: @w*@h pixels to draw
    int w, h;         //   @pixels[i * w + j] is 00RRGGBB 
    int sync;         // @sync ? sync screen : do nothing
  } _FBCtlReg;

// ---------- _DEV_SERIAL: AM Serial Controller (0000ac05) -----------

#define _DEVREG_SERIAL_RECV 0
#define _DEVREG_SERIAL_SEND 1
#define _DEVREG_SERIAL_STAT 2
#define _DEVREG_SERIAL_CTRL 3


// -------- _DEV_PCICONF: PCI Configuration Space (00000080) ---------
#define _DEVREG_PCICONF(bus, slot, func, offset) \
  ((uint32_t)(   1) << 31) | ((uint32_t)( bus) << 16) | \
  ((uint32_t)(slot) << 11) | ((uint32_t)(func) <<  8) | (offset)

// ------ _DEV_ATAx: ATA Disk Controller (00000dd0 -- 00000dd1) ------
#define _DEVREG_ATA_DATA      0
#define _DEVREG_ATA_FEATURE   1
#define _DEVREG_ATA_NSECT     2
#define _DEVREG_ATA_SECT      3
#define _DEVREG_ATA_CYLOW     4
#define _DEVREG_ATA_CYHIGH    5
#define _DEVREG_ATA_DRIVE     6
#define _DEVREG_ATA_STATUS    7

#ifdef __cplusplus
}
#endif

#endif
