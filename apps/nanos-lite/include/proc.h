#ifndef __PROC_H__
#define __PROC_H__

#include "common.h"
#include "memory.h"

#define STACK_SIZE (128 * PGSIZE)

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    _RegSet *tf;
    _Protect as;
    uintptr_t cur_brk;
    // we do not free memory, so use `max_brk' to determine whether to call mm_malloc()
    uintptr_t max_brk;
  };
} PCB;

extern PCB *current;

#endif
