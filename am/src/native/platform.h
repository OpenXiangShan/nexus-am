#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <unistd.h>

extern uintptr_t __am_rebase_offset;
#define REBASE_PTR(p) ((void *)(p) - __am_rebase_offset)
#define REBASE_ORIGINAL_VAL(x) (*(typeof(x) *)((void *)&x + __am_rebase_offset))

#endif
