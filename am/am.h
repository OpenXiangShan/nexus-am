/*
 * The Nexus Abstract Machine Architecture (AM)
 * A portable abstraction of a bare-metal computer
 */

#ifndef __AM_H__
#define __AM_H__

#include <stdint.h>
#include <stddef.h>
#include <arch.h>

#ifndef NULL
# define NULL ((void *)0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// =======================================================================
// [0] Turing Machine
// =======================================================================

typedef struct _Area {
  void *start, *end;
} _Area; 
extern _Area _heap;

void _putc(char ch);
void _halt(int code);

// =======================================================================
// [1] I/O Extension (IOE)
// =======================================================================

typedef struct _Device {
  uint32_t id;
  const char *name;
  uintptr_t (*read)(uintptr_t reg, void *buf, size_t size);
  void (*write)(uintptr_t reg, void *buf, size_t size);
} _Device;

void _ioe_init();
_Device *_device(int n);

// =======================================================================
// [2] Asynchronous Extension (ASYE)
// =======================================================================

enum {
  _EVENT_NULL = 0,
  _EVENT_IRQ_TIMER,
  _EVENT_IRQ_IODEV,
  _EVENT_PAGEFAULT,
  _EVENT_ERROR,
  _EVENT_TRAP,
  _EVENT_SYSCALL,
};
typedef struct _Event {
  int event;
  uintptr_t cause, ref;
} _Event;
typedef struct _RegSet _RegSet;

void _asye_init(_RegSet *(*l)(_Event ev, _RegSet *regs));
_RegSet *_make(_Area kstack, void *entry, void *arg);
void _yield();
void _set_intr(int enable);
int _get_intr();

// =======================================================================
// [3] Protection Extension (PTE)
// =======================================================================

#define _PROT_NONE   1
#define _PROT_READ   2
#define _PROT_WRITE  4
#define _PROT_EXEC   8
typedef struct _Protect {
  _Area area; 
  size_t pgsize;
  void *ptr;
} _Protect;

void _pte_init(void*(*pgalloc)(), void (*pgfree)(void*));
void _prot_create(_Protect *p);
void _prot_destroy(_Protect *p);
void _prot_switch(_Protect *p);
void _map(_Protect *p, void *va, void *pa);
void _protect(_Protect *p, void *va, int len, int prot);
_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args);

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

