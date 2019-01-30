#ifndef __MIPS32_H__
#define __MIPS32_H__

#ifndef __ASSEMBLER__

#include <arch.h>

#define MMIO_OFFSET(addr) ((uintptr_t)0xa0000000 + addr)

static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)MMIO_OFFSET(addr); }
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)MMIO_OFFSET(addr); }
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)MMIO_OFFSET(addr); }

static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)MMIO_OFFSET(addr) = data; }
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)MMIO_OFFSET(addr) = data; }
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)MMIO_OFFSET(addr) = data; }

#endif

#endif
