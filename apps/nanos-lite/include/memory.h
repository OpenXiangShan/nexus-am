#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"

/* the maxinum loader size is 16MB */
#define KMEM_SIZE      (16 * 1024 * 1024)

#define PGSIZE  4096
#define PGMASK  (PGSIZE - 1)

void* new_page(void);
void* kmalloc(uint32_t);

#endif
