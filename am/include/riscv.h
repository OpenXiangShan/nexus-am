#ifndef __RISCV_H__
#define __RISCV_H__

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef union {
  struct { uint32_t lo, hi; };
  int64_t val;
} R64;

static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }
static inline uint64_t ind(uintptr_t addr) { return *(volatile uint64_t *)addr; }

static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }
static inline void outd(uintptr_t addr, uint64_t data) { *(volatile uint64_t *)addr = data; }

enum { MODE_U = 0, MODE_S, MODE_H, MODE_M };
#define MSTATUS_IE(mode)  ((1 << (mode)) << 0)
#define MSTATUS_PIE(mode) ((1 << (mode)) << 4)
#define MSTATUS_MPP(mode) ((mode) << 11)
#define MSTATUS_SPP(mode) ((mode) << 8)
#define MSTATUS_MXR  (1 << 19)
#define MSTATUS_SUM  (1 << 18)
#define MSTATUS_MPRV (1 << 17)

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10

// Address in page table entry
#define PTE_ADDR(pte)   (((uintptr_t)(pte) & ~0x3ff) << 2)

#define PTW_SV32 ((ptw_config) { .ptw_level = 2, .vpn_width = 10 })
#define PTW_SV39 ((ptw_config) { .ptw_level = 3, .vpn_width = 9  })
#define PTW_SV48 ((ptw_config) { .ptw_level = 4, .vpn_width = 9  })

#endif

#endif
