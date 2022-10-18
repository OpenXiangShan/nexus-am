// cmotest.h

#ifndef __CMO_H__
#define __CMO_H__

#include <am.h>
#include <klib.h>

#if defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
#define CACHE_CTRL_BASE 0x1f10040100
#define CACHE_CMD_BASE 0x1f10040200
#define HART_CTRL_RESET_REG_BASE 0x1f10001000
#else
#define CACHE_CTRL_BASE 0x39000100
#define CACHE_CMD_BASE 0x39000200
#define HART_CTRL_RESET_REG_BASE 0x39001000
#endif
#define CMD_CMO_INV (0 + 16)
#define CMD_CMO_CLEAN (1 + 16)
#define CMD_CMO_FLUSH (2 + 16)
#define CTRL_ADDR_OFFSET 0
#define CTRL_WAY_OFFSET 8
#define CTRL_RDY_OFFSET 16 
#define CTRL_DONE_OFFSET 24
#define L3_SIZE_KB (2 * 1024)
#define L3_NR_WAY 8
#define L3_NR_BANK 4
#define OFFSET_LEN 6
#define SET_LEN 10  // log2(L3_SIZE_KB * 1024 / L3_NR_BANK / L3_NR_WAY / 64)

inline uint64_t cmo_ready() {
  volatile uint64_t busy = *(uint64_t*)(CACHE_CTRL_BASE + CTRL_RDY_OFFSET);
  return busy;
}

inline uint64_t cmo_done() {
  volatile uint64_t done = *(uint64_t*)(CACHE_CTRL_BASE + CTRL_DONE_OFFSET);
  return done;
}

void test_invalid(uint64_t paddr);
void test_clean(uint64_t paddr);
void test_flush(uint64_t paddr);

#endif
