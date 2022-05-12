#ifndef __NEMU_H__
#define __NEMU_H__

#include <klib-macros.h>

#include ISA_H // "x86.h", "mips32.h", ...

#ifdef __ARCH_X86_NEMU
# define SERIAL_PORT  0x3f8
# define KBD_ADDR     0x60
# define RTC_ADDR     0x48
# define SCREEN_ADDR  0x100
# define SYNC_ADDR    0x104
# define FB_ADDR      0xa0000000
# define AUDIO_FREQ_ADDR      0x200
# define AUDIO_CHANNELS_ADDR  0x204
# define AUDIO_SAMPLES_ADDR   0x208
# define AUDIO_SBUF_SIZE_ADDR 0x20c
# define AUDIO_INIT_ADDR      0x210
# define AUDIO_COUNT_ADDR     0x214
# define AUDIO_SBUF_ADDR      0xa0800000
#elif defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
//# define KBD_ADDR     0x40900000
# define RTC_ADDR     0x3800bff8
# define SCREEN_ADDR  0x40001000
# define SYNC_ADDR    0x40001004
# define FB_ADDR      0x50000000
#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
# define RTC_ADDR     0x1f1000bff8
// CLINT 0x1f00000000
#else
# define SERIAL_PORT  0xa10003f8
# define KBD_ADDR     0xa1000060
# define RTC_ADDR     0xa1000048
# define SCREEN_ADDR  0xa1000100
# define SYNC_ADDR    0xa1000104
# define FB_ADDR      0xa0000000
# define AUDIO_FREQ_ADDR      0xa1000200
# define AUDIO_CHANNELS_ADDR  0xa1000204
# define AUDIO_SAMPLES_ADDR   0xa1000208
# define AUDIO_SBUF_SIZE_ADDR 0xa100020c
# define AUDIO_INIT_ADDR      0xa1000210
# define AUDIO_COUNT_ADDR     0xa1000214
# define AUDIO_SBUF_ADDR      0xa0800000
#endif

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
