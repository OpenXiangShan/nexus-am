#include "common.h"

#ifndef PPU_H
#define PPU_H

void ppu_init();

int ppu_read_idx(void);

byte ppu_io_read(word address);
void ppu_io_write(word address, byte data);

static inline void W4014(byte data) {
  // DMA transfer
//  assert(ppu.OAMADDR == 0);
  extern byte PPU_SPRRAM[0x100];
  extern byte CPU_RAM[0x8000];
  void *src = &CPU_RAM[0x100 * data];
  void *dest = &PPU_SPRRAM[0];
  memcpy(dest, src, 256);
}

bool ppu_generates_nmi();
void ppu_set_mirroring(byte mirroring);

void ppu_frame();
void ppu_copy(word address, byte *source, int length);

// PPU Memory and State
typedef struct {
    byte PPUCTRL;   // $2000 write only
    int PPUMASK[8];   // $2001 write only, expanded ppu mask
    byte PPUSTATUS; // $2002 read only
    byte OAMADDR;   // $2003 write only
    byte OAMDATA;   // $2004
	word PPUSCROLL;
    byte PPUSCROLL_X, PPUSCROLL_Y; // $2005 write only x2
    word PPUADDR;   // $2006 write only x2
    word PPUDATA;   // $2007

    bool scroll_received_x;
    bool addr_received_high_byte;
    bool ready;

    int mirroring, mirroring_xor;

    int x, scanline;
} PPU_STATE;

#endif
