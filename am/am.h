/*
 * The minmal architectural-independent library for
 * implementing a system software
 *
 * Please refer to the AM specification
 */
#ifndef __AM_H__
#define __AM_H__

#include <arch.h>

typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

#define MAX_CPU 8

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Area {
  void *start, *end;
} _Area; 

// TODO: add more keys

#define _KEYS(_) \
  _(ESCAPE), _(F1), _(F2), _(F3), _(F4), _(F5), _(F6), _(F7), _(F8), _(F9), _(F10), _(F11), _(F12), \
  _(GRAVE), _(1), _(2), _(3), _(4), _(5), _(6), _(7), _(8), _(9), _(0), _(MINUS), _(EQUALS), _(BACKSPACE), \
  _(TAB), _(Q), _(W), _(E), _(R), _(T), _(Y), _(U), _(I), _(O), _(P), _(LEFTBRACKET), _(RIGHTBRACKET), _(BACKSLASH), \
  _(CAPSLOCK), _(A), _(S), _(D), _(F), _(G), _(H), _(J), _(K), _(L), _(SEMICOLON), _(RETURN), \
  _(LSHIFT), _(Z), _(X), _(C), _(V), _(B), _(N), _(M), _(COMMA), _(PERIOD), _(SLASH), _(RSHIFT), \
  _(LCTRL), _(LALT), _(APPLICATION), _(SPACE), _(RALT), _(RCTRL), \
  _(UP), _(DOWN), _(LEFT), _(RIGHT), _(INSERT), _(DELETE), _(HOME), _(END), _(PAGEUP), _(PAGEDOWN)

#define KEY_NAME(k) _KEY_##k

enum {
  _KEY_NONE = 0,
  _KEYS(KEY_NAME),
};

typedef struct _Screen {
  int width, height;
} _Screen;

typedef struct _Protect {
  _Area area; 
  void *ptr;
} _Protect;

// =======================================================================
// [0] Turing Machine: code execution & a heap memory
// =======================================================================

void _start();
void _trm_init();
void _putc(char ch);
void _halt(int code);
extern _Area _heap;

// =======================================================================
// [1] IO Extension
// =======================================================================

void _ioe_init();
ulong _uptime();
ulong _cycles();
int _peek_key();
typedef u32 _Pixel;
void _draw_p(int x, int y, _Pixel p);
void _draw_f(_Pixel *p); 
void _draw_sync();
extern _Screen _screen;
// TODO: bus interfaces

// =======================================================================
// [2] Asynchronous Extension
// =======================================================================

void _asye_init();
void _listen(_RegSet* (*l)(int ex, _RegSet *regs));
_RegSet *_make(_Area kstack, void *entry);
void _trap();
void _idle();
void _ienable();
void _idisable();
int _istatus();

// =======================================================================
// [3] Protection Extension
// =======================================================================

void _pte_init(void*(*palloc)(), void (*pfree)(void*));
void _protect(_Protect *p);
void _release(_Protect *p);
void _map(_Protect *p, void *va, void *pa);
void _unmap(_Protect *p, void *va);
void _switch(_Protect *p);
_RegSet *_umake(_Area ustack, _Area kstack, void *entry, int argc, char **argv);

// =======================================================================
// [4] Multi-Processor Extension
// =======================================================================

void _mpe_init(void (*entry)());
int _cpu();
ulong _atomic_xchg(volatile ulong *addr, ulong newval);
void _barrier();
extern int _NR_CPU;

#ifdef __cplusplus
}
#endif

#endif

