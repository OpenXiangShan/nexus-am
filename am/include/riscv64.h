#ifndef __RISCV64_H__
#define __RISCV64_H__

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef union {
  struct { uint32_t lo, hi; };
  int64_t val;
} R64;

#define true 1
#define false 0

static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }
static inline uint64_t ind(uintptr_t addr) { return *(volatile uint64_t *)addr; }

static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }
static inline void outd(uintptr_t addr, uint64_t data) { *(volatile uint64_t *)addr = data; }

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10

typedef uintptr_t PTE;
#define NR_PTE    512     // # PTEs per page table
#define PGSHFT    12      // log2(PGSIZE)
#define PN(addr)    ((uint64_t)(addr) >> PGSHFT)
#define OFF(va)     ((uint64_t)(va) & (PGSIZE - 1))

// Sv39 page walk
#define PTW_LEVEL 3
#define PTE_SIZE 8
#define VPNMASK 0x1ff
// Offset of VPNi in a virtual address
static inline uintptr_t VPNiSHFT(int i) {
  return (PGSHFT) + 9 * i;
}
static inline uintptr_t VPNi(uintptr_t va, int i) {
  return (va >> VPNiSHFT(i)) & VPNMASK;
}

// Address in page table entry
#define PTE_ADDR(pte)   (((uint64_t)(pte) & ~0x3ff) << 2)

#endif

#endif
