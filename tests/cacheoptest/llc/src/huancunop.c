#include "huancunop.h"

void wait(int cycle) {
  volatile int i = cycle;
  while(i > 0){
    i--;
  }
}

void make_invalid(uint64_t paddr) {
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(paddr);
  *(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(paddr);
  (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_INV;
}

void make_clean(uint64_t paddr) {
*(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(paddr);
*(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(paddr);
(*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_CLEAN;
}

void make_flush(uint64_t paddr) {
*(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(paddr);
*(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(paddr);
(*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_FLUSH;
}

// Flush an n*512 bit address region to memory
void make_region_invalid(uint64_t start_paddr, uint64_t size_in_byte) {
  for(uint64_t current_paddr = start_paddr; current_paddr < (start_paddr + size_in_byte); current_paddr += CACHE_LINE_SIZE_BYTE) {
    if(is_ready()) {
      *(uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(current_paddr);
      *(uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(current_paddr);
      (*(uint64_t*)CACHE_CMD_BASE) = CMD_CMO_INV;   // or CMD_CMO_CLEAN / CMD_CMO_FLUSH
    } else {
      continue;
    }
  }
}

