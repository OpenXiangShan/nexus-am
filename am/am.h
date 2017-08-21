/*
 * The Nexus Abstract Architecture
 * Minmal architectural-independent library for implementing system software
 *
 * Please refer to the AM specification
 */
#ifndef __AM_H__
#define __AM_H__

#include <stdint.h>
#include <sys/types.h>
#include <arch.h>

#ifndef NULL
# define NULL ((void *)0)
#endif

typedef struct _Area {
  void *start, *end;
} _Area; 

#ifdef __cplusplus
extern "C" {
#endif

// =======================================================================
// [0] Turing Machine: code execution & a heap memory
// =======================================================================

void _putc(char ch);
void _halt(int code);
extern _Area _heap;

// =======================================================================
// [1] IO Extension (IOE)
// =======================================================================

typedef struct _Device {
  uint32_t id;
  const char *name;
  intptr_t (*read)(intptr_t reg, size_t nmemb);
  void (*write)(intptr_t reg, size_t nmemb, intptr_t data);
} _Device;

void _ioe_init();
_Device *_device(int n);

// =======================================================================
// [2] Asynchronous Extension (ASYE)
// =======================================================================

#define _EVENTS(_) \
  _(IRQ_TIME) _(IRQ_IODEV) \
  _(ERROR) _(PAGE_FAULT) _(BUS_ERROR) _(NUMERIC) \
  _(TRAP) _(SYSCALL)

#define _EVENT_NAME(ev) _EVENT_##ev,

enum {
  _EVENT_NULL = 0,
  _EVENTS(_EVENT_NAME)
};

typedef struct _RegSet _RegSet;

typedef struct _Event {
  int event;
  intptr_t cause;
} _Event;

void _asye_init(_RegSet* (*l)(_Event ev, _RegSet *regs));
_RegSet *_make(_Area kstack, void *entry, void *arg);
void _trap();
int _istatus(int enable);

// =======================================================================
// [3] Protection Extension (PTE)
// =======================================================================

typedef struct _Protect {
  _Area area; 
  void *ptr;
} _Protect;

void _pte_init(void*(*palloc)(), void (*pfree)(void*));
void _protect(_Protect *p);
void _release(_Protect *p);
void _map(_Protect *p, void *va, void *pa);
void _unmap(_Protect *p, void *va);
void _switch(_Protect *p);
_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]);

// =======================================================================
// [4] Multi-Processor Extension (MPE)
// =======================================================================

void _mpe_init(void (*entry)());
int _cpu();
intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval);
void _barrier();
extern int _NR_CPU;

#ifdef __cplusplus
}
#endif

#endif

