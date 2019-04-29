#ifndef MEM_H
#define MEM_H

#include "common.h"
#include "mmc.h"
#include <memory.h>
#include <cpu.h>
#include <ppu.h>
#include <psg.h>

static inline uint32_t memory_readb(uint32_t address) {
  int idx = address >> 13;
  if (idx == 0) { return cpu_ram_read(address); }
  else if (idx == 1) { return ppu_io_read(address); }
  else if (idx > 3) { return mmc_read(address); }
  else if (idx == 2) { return psg_io_read(address); }
  else if (idx == 3) { return cpu_ram_read(address); }
  else { assert(0); }
}

static inline uint32_t instr_fetch(uint32_t address) {
  extern byte memory[0x10000]; // mmc

  // for super mairo, all fetch are from mmc
  return memory[address];
}

static inline void memory_writeb(uint32_t address, uint32_t byte_data) {
  int idx = address >> 13;
  if (idx == 0) { cpu_ram_write(address, byte_data); }
  else if (idx == 1) { ppu_io_write(address, byte_data); }
  else if (idx == 2) {
    if (address == 0x4014) W4014(byte_data);
    else psg_io_write(address, byte_data);
  }
  else if (idx == 3) { cpu_ram_write(address, byte_data); }
  else {
    // for super mario, it does not write to mmc
    assert(0);
  }
}

static inline uint32_t memory_readw(uint32_t address)
{
    return memory_readb(address) | (memory_readb(address + 1) << 8);
}

static inline uint32_t instr_fetchw(uint32_t address)
{
    return instr_fetch(address) | (instr_fetch(address + 1) << 8);
}

static inline void memory_writew(uint32_t address, uint32_t word_data)
{
    memory_writeb(address, word_data & 0xFF);
    memory_writeb(address + 1, word_data >> 8);
}
#endif
