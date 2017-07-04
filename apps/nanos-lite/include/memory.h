#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

#define PGMASK          (PGSIZE - 1)    // Mask for bit ops
#define PGROUNDUP(sz)   (((sz)+PGSIZE-1) & ~PGMASK)
#define PGROUNDDOWN(a)  (((a)) & ~PGMASK)

void* new_page(void);
void* kmalloc(uint32_t);

#endif
