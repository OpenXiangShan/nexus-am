#include "common.h"

#ifndef PPU_H
#define PPU_H

extern byte PPU_RAM[0x4000];

void ppu_init();
void ppu_finish();

int ppu_read_idx(void);

byte ppu_io_read(word address);
void ppu_io_write(word address, byte data);

bool ppu_generates_nmi();
void ppu_set_generates_nmi(bool yesno);

void ppu_set_mirroring(byte mirroring);

//void ppu_run(int cycles);
void ppu_cycle();
int ppu_scanline();
void ppu_set_scanline(int s);
void ppu_copy(word address, byte *source, int length);
void ppu_sprram_write(byte data);

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

extern PPU_STATE ppu;
extern byte ppu_latch;
extern bool ppu_sprite_hit_occured;


// Screen State and Rendering

static inline byte ppu_l_h_addition(int h, int l, int x) {
  return (((h >> (7 - x)) & 1) << 1) | ((l >> (7 - x)) & 1);
}
static inline byte ppu_l_h_addition_flip(int l, int h, int x) {
  return (((h >> x) & 1) << 1) | ((l >> x) & 1);
}
 

// Draws current screen pixels in ppu_background_pixels & ppu_sprite_pixels and clears them
void ppu_render_screen();
void ppu_set_background_color(byte color);

#endif
