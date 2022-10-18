// cmotest.c

#include "cmotest.h"

void test_invalid(uint64_t paddr) {
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_ADDR_OFFSET) = paddr;
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_INV;
}

void test_clean(uint64_t paddr) {
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_ADDR_OFFSET) = paddr;
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_CLEAN;
}

void test_flush(uint64_t paddr) {
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_ADDR_OFFSET) = paddr;
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_FLUSH;
}
