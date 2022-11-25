#include <am.h>
#include <klib.h>

#if defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
#define CACHE_CTRL_BASE 0x1f10040100
#define CACHE_RDY_REG 0x1f10040180
#define CACHE_CMD_BASE 0x1f10040200
#define HART_CTRL_RESET_REG_BASE 0x1f10001000
#else
#define CACHE_CTRL_BASE 0x39000100
#define CACHE_RDY_REG 0x39000180
#define CACHE_CMD_BASE 0x39000200
#define HART_CTRL_RESET_REG_BASE 0x39001000
#endif
#define CMD_CMO_INV (0 + 16)
#define CMD_CMO_CLEAN (1 + 16)
#define CMD_CMO_FLUSH (2 + 16)
#define CTRL_TAG_OFFSET 0
#define CTRL_SET_OFFSET 8
#define CTRL_WAY_OFFSET 16
#define CTRL_DATA_OFFSET 24
#define L3_SIZE_KB (2 * 1024)
#define L3_NR_WAY 8
#define L3_NR_BANK 4
#define SET_SIZE 1024   // L3_SIZE_KB * 1024 / L3_NR_BANK / L3_NR_WAY / 64
#define SET_LEN 10      // log2(SET_SIZE)
#define OFFSET_LEN 6
#define CACHE_LINE_SIZE_BIT 512
#define CACHE_LINE_SIZE_BYTE (CACHE_LINE_SIZE_BIT / 8)

void wait(int);
void wait_for_ready();

inline uint64_t is_ready() {
  return *((uint64_t*)CACHE_RDY_REG);
}

inline uint64_t get_tag(uint64_t paddr) {
  return (paddr >> OFFSET_LEN) >> SET_LEN;
}

inline uint64_t get_set(uint64_t paddr) {
  return (paddr >> OFFSET_LEN) & (SET_SIZE-1);
}

// regular flow to issue a CMO
void make_invalid(uint64_t);
void make_clean(uint64_t);
void make_flush(uint64_t);

// operate on a memory region
void make_region_invalid(uint64_t, uint64_t);
void make_region_clean(uint64_t, uint64_t);
void make_region_flush(uint64_t, uint64_t);
