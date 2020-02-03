#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <am.h>
#include <unistd.h>
#include <signal.h>
#include <klib.h>

// the size of red zone of the stack frame, see the amd64 ABI manual for details
#define RED_NONE_SIZE 128

void *__am_private_alloc(size_t n);
void __am_get_example_uc(_Context *r);
void __am_get_intr_sigmask(sigset_t *s);
int __am_is_sigmask_sti(sigset_t *s);
void __am_init_irq();
void __am_shm_mmap(void *va, void *pa, int prot);
void __am_shm_munmap(void *va);

#endif
