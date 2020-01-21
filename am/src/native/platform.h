#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <am.h>
#include <unistd.h>
#include <signal.h>
#include <klib.h>

void *__am_private_alloc(size_t n);
void __am_get_example_uc(_Context *r);
void __am_init_irq();
void __am_shm_mmap(void *va, void *pa, int prot);
void __am_shm_munmap(void *va);

#endif
