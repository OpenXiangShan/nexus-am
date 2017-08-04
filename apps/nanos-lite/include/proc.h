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
  };
} PCB;

extern PCB *current;

#endif
