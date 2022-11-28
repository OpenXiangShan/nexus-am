#include "huancunop.h"

void wait(int cycle) {
  volatile int i = cycle;
  while(i > 0){
    i--;
  }
}

void wait_for_ready() {
  volatile uint64_t cycle = 0;
  while(1) {
    if(is_ready()) {
      break;
    } else if(cycle >= 200) {
      printf("wait for too long.");
      _halt(-1);
    } else {
      cycle++;
    }
  }
}

void make_invalid(uint64_t paddr) {
  *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(paddr);
  *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(paddr);
  *(volatile uint64_t*)CACHE_CMD_BASE = CMD_CMO_INV;
}

void make_clean(uint64_t paddr) {
  *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(paddr);
  *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(paddr);
  *(volatile uint64_t*)CACHE_CMD_BASE = CMD_CMO_CLEAN;
}

void make_flush(uint64_t paddr) {
  *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(paddr);
  *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(paddr);
  *(volatile uint64_t*)CACHE_CMD_BASE = CMD_CMO_FLUSH;
}

void make_region_invalid(uint64_t start_paddr, uint64_t size_in_byte) {
  for(uint64_t current_paddr = start_paddr; current_paddr < (start_paddr + size_in_byte); current_paddr += CACHE_LINE_SIZE_BYTE) {
    wait_for_ready();
    *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(current_paddr);
    *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(current_paddr);
    *(volatile uint64_t*)CACHE_CMD_BASE = CMD_CMO_INV;
  }
}

void make_region_clean(uint64_t start_paddr, uint64_t size_in_byte) {
  for(uint64_t current_paddr = start_paddr; current_paddr < (start_paddr + size_in_byte); current_paddr += CACHE_LINE_SIZE_BYTE) {
    wait_for_ready();
    *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(current_paddr);
    *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(current_paddr);
    *(volatile uint64_t*)CACHE_CMD_BASE = CMD_CMO_CLEAN;
  }
}

void make_region_flush(uint64_t start_paddr, uint64_t size_in_byte) {
  for(uint64_t current_paddr = start_paddr; current_paddr < (start_paddr + size_in_byte); current_paddr += CACHE_LINE_SIZE_BYTE) {
    wait_for_ready();
    *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_TAG_OFFSET) = get_tag(current_paddr);
    *(volatile uint64_t*)(CACHE_CTRL_BASE + CTRL_SET_OFFSET) = get_set(current_paddr);
    *(volatile uint64_t*)CACHE_CMD_BASE = CMD_CMO_FLUSH;
  }
}
