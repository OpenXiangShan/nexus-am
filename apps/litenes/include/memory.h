#ifndef MEM_H
#define MEM_H

#include "common.h"
#include "mmc.h"
#include <memory.h>
#include <cpu.h>
#include <ppu.h>
#include <psg.h>

static inline uint32_t memory_readb(uint32_t address)
{
    switch (address >> 13) {
        case 0:
        case 3: return cpu_ram_read(address);
        case 1: return ppu_io_read(address);
        case 2: return psg_io_read(address);
        default: return mmc_read(address);
    }
}

static inline uint32_t instr_fetch(uint32_t address) {
  extern byte memory[0x10000]; // mmc
  extern byte CPU_RAM[0x8000]; // CPU Memory

  if ((address >> 15) == 0) {
    return CPU_RAM[address & 0x7FF];
  }
  else {
    return memory[address];
  }
}

static inline void memory_writeb(uint32_t address, uint32_t byte_data)
{
    switch (address >> 13) {
        case 0:
        case 3: return cpu_ram_write(address, byte_data);
        case 1: return ppu_io_write(address, byte_data);
        case 2:
          if (address == 0x4014) {
            // DMA transfer
            int i;
              for (i = 0; i < 256; i++) {
                  ppu_sprram_write(cpu_ram_read((0x100 * byte_data) + i));
              }
              return;
          }
          return psg_io_write(address, byte_data);
        default: return mmc_write(address, byte_data);
    }
}

static inline uint32_t memory_readw(uint32_t address)
{
    return memory_readb(address) + (memory_readb(address + 1) << 8);
}

static inline void memory_writew(uint32_t address, uint32_t word_data)
{
    memory_writeb(address, word_data & 0xFF);
    memory_writeb(address + 1, word_data >> 8);
}
#endif
