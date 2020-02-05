#include "common.h"

static inline uint32_t mmc_read(word address) {
  extern byte memory[0x10000];
  return memory[address];
}

void mmc_write(word address, byte data);
void mmc_copy(word address, byte *source, int length);
void mmc_append_chr_rom_page(byte *source);

extern byte mmc_id;
