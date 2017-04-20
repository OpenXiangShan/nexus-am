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
enum {
  _KEY_NONE = 0,
  _KEY_UP, _KEY_DOWN, _KEY_LEFT, _KEY_RIGHT,
  _KEY_Z, _KEY_X,
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

