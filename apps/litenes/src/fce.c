#include "fce.h"
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "hal.h"
#include "nes.h"


typedef struct {
    char signature[4];
    byte prg_block_count;
    byte chr_block_count;
    word rom_type;
    byte reserved[8];
} ines_header;

static ines_header fce_rom_header;

// FCE Lifecycle

void
romread(char *rom, void *buf, int size)
{
    static int off = 0;
    memcpy(buf, rom + off, size);
    off += size;
}

int fce_load_rom(char *rom)
{
    romread(rom, &fce_rom_header, sizeof(fce_rom_header));

    if (memcmp(fce_rom_header.signature, "NES\x1A", 4)) {
        return -1;
    }

    mmc_id = ((fce_rom_header.rom_type & 0xF0) >> 4);

    int prg_size = fce_rom_header.prg_block_count * 0x4000;
    static byte buf[0x10000];
    romread(rom, buf, prg_size);

    if (mmc_id == 0 || mmc_id == 3) {
        // if there is only one PRG block, we must repeat it twice
        if (fce_rom_header.prg_block_count == 1) {
            mmc_copy(0x8000, buf, 0x4000);
            mmc_copy(0xC000, buf, 0x4000);
        }
        else {
            mmc_copy(0x8000, buf, 0x8000);
        }
    }
    else {
        return -1;
    }

    // Copying CHR pages into MMC and PPU
    int i;
    for (i = 0; i < fce_rom_header.chr_block_count; i++) {
        romread(rom, buf, 0x2000);
        mmc_append_chr_rom_page(buf);

        if (i == 0) {
            ppu_copy(0x0000, buf, 0x2000);
        }
    }

    return 0;
}

void fce_init()
{
    cpu_init();
    ppu_init();
    ppu_set_mirroring(fce_rom_header.rom_type & 1);
    cpu_reset();
}

static ulong time;

void wait_for_frame() {
  ulong cnt = _uptime();
  while (cnt - time < 1000 / FPS) {
    cnt = _uptime();
  }
  time = cnt;
}


int key_state[256];

void fce_run()
{
    key_state[0] = 1;
    time = _uptime();
    while(1)
    {
        wait_for_frame();
        int scanlines = 262;
        while (scanlines-- > 0)
        {
            ppu_run(1);
            cpu_run(1364 / 12); // 1 scanline
        }

        int key = _peek_key();
        if (key != _KEY_NONE) {
          int down = (key & 0x8000) != 0;
          int code = key & ~0x8000;
          key_state[code] = down;
        }
    }
}

// Rendering

byte canvas[W][H];

void fce_update_screen()
{
  int idx = ppu_ram_read(0x3F00);

  int w = _screen.width;
  int h = _screen.height;

  int pad = (w - h) / 2;
  for (int x = pad; x < w - pad; x ++) {
    for (int y = 0; y < h; y ++) {
      _draw_p(x, y, palette[canvas[(x - pad) * W / h][y * H / h]]);
    }
  }

  _draw_sync();

  for (int i = 0; i < W; i ++)
    for (int j = 0; j < H; j ++)
      canvas[i][j] = idx;
}

extern char mario_nes[];

int main() {
  _trm_init();
  _ioe_init();

  fce_load_rom(mario_nes);
  fce_init();
  fce_run();
  return 1;
}

static char *alloc_head;

void *halloc(size_t size) {
  if (alloc_head == 0) {
    alloc_head = (char*)_heap.start;
  }
  while ((ulong)alloc_head % 16 != 0) alloc_head ++;
  alloc_head += size;
  if (alloc_head >= (char*)_heap.end) return 0;
  return alloc_head - size;
}
