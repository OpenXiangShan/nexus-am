/*
 * The Nexus Abstract Machine Architecture (AM)
 * A portable abstraction of a bare-metal computer
 */

#ifndef __AM_H__
#define __AM_H__

#include <stdint.h>
#include <stddef.h>
#include <arch.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===================== Constants and Structs =======================

enum {
  _EVENT_NULL = 0,
  _EVENT_ERROR,
  _EVENT_IRQ_TIMER,
  _EVENT_IRQ_IODEV,
  _EVENT_PAGEFAULT,
  _EVENT_YIELD,
  _EVENT_SYSCALL,
};

#define _PROT_NONE   1    // no access
#define _PROT_READ   2    // can read
#define _PROT_WRITE  4    // can write
#define _PROT_EXEC   8    // can execute

// Memory area for [@start, @end)
typedef struct _Area {
  void *start, *end;
} _Area; 

// A device (@id, @name) with @read/@write support
// See <amdev.h> for device descriptions
typedef struct _Device {
  uint32_t id;
  const char *name;
  size_t (*read) (uintptr_t reg, void *buf, size_t size);
  size_t (*write)(uintptr_t reg, void *buf, size_t size);
} _Device;

// An event of type @event, caused by @cause of pointer @ref
typedef struct _Event {
  int event;
  uintptr_t cause, ref;
  const char *msg;
} _Event;
typedef struct _Context _Context;

// A protected address space with user memory @area
// and arch-dependent @ptr
typedef struct _Protect {
  size_t pgsize;
  _Area area;
  void *ptr;
} _Protect;

// ====================== Turing Machine (TRM) =======================

extern _Area _heap;
void _putc(char ch);
void _halt(int code) __attribute__((__noreturn__));

// ======================= I/O Extension (IOE) =======================

int _ioe_init();
_Device *_device(int n);

// ====================== Context Extension (CTE) ====================

int _cte_init(_Context *(*handler)(_Event ev, _Context *ctx));
void _yield();
int _intr_read();
void _intr_write(int enable);
_Context *_kcontext(_Area kstack, void (*entry)(void *), void *arg);

// ================= Virtual Memory Extension (VME) ==================

int _vme_init(void *(*pgalloc)(size_t size), void (*pgfree)(void *));
int _protect(_Protect *p);
void _unprotect(_Protect *p);
int _map(_Protect *p, void *va, void *pa, int prot);
_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack,
                                 void *entry, void *args);

// ================= Multi-Processor Extension (MPE) =================

int _mpe_init(void (*entry)());
int _ncpu();
int _cpu();
intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval);

#ifdef __cplusplus
}
#endif

#endif
