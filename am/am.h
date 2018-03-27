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

// ===================== Constants and Structs =======================

enum {
  _EVENT_NULL = 0,
  _EVENT_IRQ_TIMER,
  _EVENT_IRQ_IODEV,
  _EVENT_PAGEFAULT,
  _EVENT_ERROR,
  _EVENT_YIELD,
  _EVENT_SYSCALL,
};

#define _PROT_NONE   1    // no access
#define _PROT_READ   2    // can read
#define _PROT_WRITE  4    // can write
#define _PROT_EXEC   8    // can execute

// A memory area of [@start, @end)
typedef struct _Area {
  void *start, *end;
} _Area; 

// A device (@id, @name) with @read/@write support
// See <amdev.h> for device descriptions
typedef struct _Device {
  uint32_t id;
  const char *name;
  size_t (*read)(uintptr_t reg, void *buf, size_t size);
  size_t (*write)(uintptr_t reg, void *buf, size_t size);
} _Device;

// An event of type @event, caused by @cause of pointer @ref
typedef struct _Event {
  int event;
  uintptr_t cause, ref;
} _Event;
typedef struct _RegSet _RegSet;

// A protected address space with user memory @area
// and arch-dependent @ptr
typedef struct _Protect {
  _Area area; 
  void *ptr;
} _Protect;

// ========================= Turing Machine ==========================

extern _Area _heap;
void _putc(char ch);
void _halt(int code);

// ======================= I/O Extension (IOE) =======================

int _ioe_init();
_Device *_device(int n);

// ================== Asynchronous Extension (ASYE) ==================

int _asye_init(_RegSet *(*handler)(_Event ev, _RegSet *regs));
_RegSet *_make(_Area kstack, void (*entry)(void *), void *arg);
void _yield();
int _intr_read();
void _intr_write(int enable);

// =================== Protection Extension (PTE) ====================

int _pte_init(void *(*pgalloc)(size_t npages), void (*pgfree)(void *));
int _prot_create(_Protect *p);
void _prot_destroy(_Protect *p);
void _prot_switch(_Protect *p);
int _map(_Protect *p, void *va, void *pa);
int _protect(_Protect *p, void *va, int len, int prot);
_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack,
                void (*entry)(void *), void *args);

// ================= Multi-Processor Extension (MPE) =================

int _mpe_init(void (*entry)());
int _ncpu();
int _cpu();
intptr_t _atomic_xchg(volatile intptr_t *addr, intptr_t newval);

#ifdef __cplusplus
}
#endif

#endif