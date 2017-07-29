#ifndef __ARCH_H__
#define __ARCH_H__

#include <am.h>

#define PMEM_SIZE (64 * 1024 * 1024)
#define PGSIZE    4096    // Bytes mapped by a page

typedef uint32_t size_t;

struct _RegSet {
  uintptr_t edi, esi, ebp, esp;
  uintptr_t ebx, edx, ecx, eax;      // Register saved by pushal
  int       irq;                     // # of irq
  uintptr_t err, eip, cs, eflags;    // Execution state before trap
};

typedef struct _RegSet TrapFrame;

static inline uint8_t inb(int port) {
  char data;
  asm volatile("inb %1, %0" : "=a"(data) : "d"((uint16_t)port));
  return data;
}

static inline uint32_t inl(int port) {
  long data;
  asm volatile("inl %1, %0" : "=a"(data) : "d"((uint16_t)port));
  return data;
}

static inline void outb(int port, uint8_t data) {
  asm volatile("outb %%al, %%dx" : : "a"(data), "d"((uint16_t)port));
}

static inline void outl(int port, uint32_t data) {
  asm volatile("outl %%eax, %%dx" : : "a"(data), "d"((uint16_t)port));
}

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
