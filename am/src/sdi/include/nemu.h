#ifndef __NEMU_H__
#define __NEMU_H__

#include <klib-macros.h>

# define SERIAL_PORT  0xa10003f8
# define KBD_ADDR     0xa1000060
# define RTC_ADDR     0xa1000048
# define SCREEN_ADDR  0xa1000100
# define SYNC_ADDR    0xa1000104
# define FB_ADDR      0xa0000000

#define MMIO_BASE 0xa0000000
#define MMIO_SIZE 0x10000000

static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }

static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }

extern char _pmem_start, _pmem_end;

#define NEMU_PADDR_SPACE \
  RANGE(&_pmem_start, &_pmem_end), \
  RANGE(0xa0000000, 0xa0000000 + 0x80000), /* vmem */ \
  RANGE(0xa1000000, 0xa1000000 + 0x1000)   /* serial, rtc, screen, keyboard */

#define PGSIZE    4096
#define PGSHFT    12      // log2(PGSIZE)
#define PN(addr)    ((uintptr_t)(addr) >> PGSHFT)
#define OFF(va)     ((uintptr_t)(va) & (PGSIZE - 1))

typedef uintptr_t PTE;

typedef struct {
  int ptw_level;
  int vpn_width;
} ptw_config;

// Offset of VPN[i] in a virtual address
static inline int VPNiSHFT(const ptw_config c, int i) {
  return (PGSHFT) + c.vpn_width * i;
}
// Extract the VPN[i] field in a virtual address
static inline uintptr_t VPNi(const ptw_config c, uintptr_t va, int i) {
  uintptr_t vpn_mask = (1 << c.vpn_width) - 1;
  return (va >> VPNiSHFT(c, i)) & vpn_mask;
}

#endif
