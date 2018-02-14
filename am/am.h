/*
 * The Nexus Abstract Machine Architecture (AM)
 * A minimal architectural-independent library for implementing system software
 */

#ifndef __AM_H__
#define __AM_H__

#include <stdint.h>
#include <stddef.h>
#include <arch.h>

#ifndef NULL
# define NULL ((void *)0)
#endif

typedef struct _Area {
  void *start, *end;
} _Area; 

typedef struct _Device {
  uint32_t id;
  const char *name;
  uintptr_t (*read)(uintptr_t reg, size_t nmemb);
  void (*write)(uintptr_t reg, size_t nmemb, uintptr_t data);
} _Device;

typedef struct _RegSet _RegSet;

enum {
  _EVENT_NULL = 0,
  _EVENT_IRQ_TIMER, _EVENT_IRQ_IODEV,
  _EVENT_ERROR,
  _EVENT_NUMERIC,
  _EVENT_PAGE_FAULT,
  _EVENT_TRAP, _EVENT_SYSCALL,
};

typedef struct _Event {
  int event;
  uintptr_t cause, ref;
} _Event;

typedef struct _Protect {
  _Area area; 
  void *ptr;
} _Protect;

#ifdef __cplusplus
extern "C" {
#endif

// =======================================================================
// [0] Turing Machine
// =======================================================================

extern _Area _heap;
void _putc(char ch);
void _halt(int code);

// =======================================================================
// [1] I/O Extension (IOE)
// =======================================================================

void _ioe_init();
_Device *_device(int n);

// =======================================================================
// [2] Asynchronous Extension (ASYE)
// =======================================================================

void _asye_init(_RegSet *(*l)(_Event ev, _RegSet *regs));
_RegSet *_make(_Area kstack, void *entry, void *arg);
void _trap();
int _istatus(int enable);

// =======================================================================
// [3] Protection Extension (PTE)
// =======================================================================

void _pte_init(void*(*palloc)(), void (*pfree)(void*));
void _protect(_Protect *p);
void _release(_Protect *p);
void _map(_Protect *p, void *va, void *pa, uint8_t mode);
void _unmap(_Protect *p, void *va);
void _switch(_Protect *p);
_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, void *arg);

// =======================================================================
// [4] Multi-Processor Extension (MPE)
// =======================================================================

void _mpe_init(void (*entry)());
int _ncpu();
int _cpu();
intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval);

#ifdef __cplusplus
}
#endif

#endif

