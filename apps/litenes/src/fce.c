#include <fce.h>
#include <cpu.h>
#include <memory.h>
#include <ppu.h>
#include <klib.h>
#include <amdev.h>

// #define NOGUI

int key_state[256];
bool do_update = false;
static byte *buf;

typedef struct {
    char signature[4];
    byte prg_block_count;
    byte chr_block_count;
    word rom_type;
    byte reserved[8];
} ines_header;

static ines_header *fce_rom_header;

// FCE Lifecycle

byte *romread(int size)
{
    byte *ret = buf;
    buf += size;
    return ret;
}

int fce_load_rom(char *rom)
{
    buf = (byte*)rom;
    fce_rom_header = (ines_header*)romread(sizeof(ines_header));

    if (memcmp(fce_rom_header->signature, "NES\x1A", 4)) {
        return -1;
    }

    mmc_id = ((fce_rom_header->rom_type & 0xF0) >> 4);

    int prg_size = fce_rom_header->prg_block_count * 0x4000;
    
    byte *blk = romread(prg_size);

    if (mmc_id == 0 || mmc_id == 3) {
        // if there is only one PRG block, we must repeat it twice
        if (fce_rom_header->prg_block_count == 1) {
            mmc_copy(0x8000, blk, 0x4000);
            mmc_copy(0xC000, blk, 0x4000);
        }
        else {
            mmc_copy(0x8000, blk, 0x8000);
        }
    }
    else {
        return -1;
    }

    // Copying CHR pages into MMC and PPU
    int i;
    for (i = 0; i < fce_rom_header->chr_block_count; i++) {
        byte *blk = romread(0x2000);
        mmc_append_chr_rom_page(blk);

        if (i == 0) {
            ppu_copy(0x0000, blk, 0x2000);
        }
    }

    return 0;
}

void fce_init()
{
	log("call cpu_init\n");
    cpu_init();
	log("call ppu_init\n");
    ppu_init();
	log("call ppu_set_mirroring\n");
    ppu_set_mirroring(fce_rom_header->rom_type & 1);
	log("call cpu_reset\n");
    cpu_reset();
}

static unsigned long frames;

void wait_for_frame() {
  frames++;
#ifdef NOGUI
  return;
#endif
  while (1) {
    unsigned long expect = uptime() / (1000 / FPS);
    if (expect >= frames) break;
  }
}

void fce_run()
{
    key_state[0] = 1;
    int nr_draw = 0;
    uint32_t last = uptime();
    while(1)
    {
        wait_for_frame();

        ppu_frame();

        int key = read_key();
        for (; key != _KEY_NONE; key = read_key()) {
          int down = (key & 0x8000) != 0;
          int code = key & ~0x8000;
          key_state[code] = down;
        }

        nr_draw ++;
        if (uptime() - last > 1000) {
          last = uptime();
          printf("FPS = %d\n", nr_draw);
          nr_draw = 0;
        }
    }
}

// Rendering

const uint32_t palette[64] = {
  0x808080, 0x0000BB, 0x3700BF, 0x8400A6, 0xBB006A, 0xB7001E,
  0xB30000, 0x912600, 0x7B2B00, 0x003E00, 0x00480D, 0x003C22,
  0x002F66, 0x000000, 0x050505, 0x050505, 0xC8C8C8, 0x0059FF,
  0x443CFF, 0xB733CC, 0xFF33AA, 0xFF375E, 0xFF371A, 0xD54B00,
  0xC46200, 0x3C7B00, 0x1E8415, 0x009566, 0x0084C4, 0x111111,
  0x090909, 0x090909, 0xFFFFFF, 0x0095FF, 0x6F84FF, 0xD56FFF,
  0xFF77CC, 0xFF6F99, 0xFF7B59, 0xFF915F, 0xFFA233, 0xA6BF00,
  0x51D96A, 0x4DD5AE, 0x00D9FF, 0x666666, 0x0D0D0D, 0x0D0D0D,
  0xFFFFFF, 0x84BFFF, 0xBBBBFF, 0xD0BBFF, 0xFFBFEA, 0xFFBFCC,
  0xFFC4B7, 0xFFCCAE, 0xFFD9A2, 0xCCE199, 0xAEEEB7, 0xAAF7EE,
  0xB3EEFF, 0xDDDDDD, 0x111111, 0x111111
}; 

#ifdef STRETCH
byte canvas[257][520];
static int xmap[1024];
static uint32_t row[1024];
#else
// add align attribute here to enable fast memcpy
uint32_t screen[H][W + 16] __attribute((aligned(8)));
#endif

void fce_update_screen() {
#ifdef NOGUI
  return;
#elif defined(__ISA_NATIVE__)
  do_update = 1;
#else
  static int frame_cnt;
  do_update = (frame_cnt == 0);
  frame_cnt ++;
  if (frame_cnt != 2) return;
  frame_cnt = -1;
#endif

  int w = screen_width();
  int h = screen_height();

  int idx = ppu_read_idx();

#ifdef STRETCH
  int pad = (w - h) / 2;
  for (int y = 0; y < h; y ++) {
    int y1 = y * (H - 1) / h + 1;
    for (int x = pad; x < w - pad; x ++) {
      row[x] = palette[canvas[y1][xmap[x]]];
    }
    draw_rect(row + pad, pad, y, w - 2 * pad, 1);
  }

  assert(sizeof(byte) == 1);
  memset(canvas, idx, sizeof(canvas));
#else
  int xpad = (w - W) / 2;
  int ypad = (h - H) / 2;
  assert(xpad >= 0 && ypad >= 0);

  for (int y = 0; y < H; y ++) {
    draw_rect(&screen[y][8], xpad, ypad + y, W, 1);
  }

//  draw_rect(&screen[0][0], xpad, ypad, W, H);

  int nr64 = sizeof(screen[0][0]) * W / sizeof(uint64_t);
  int i;
  uint64_t v = ((uint64_t)palette[idx] << 32) | palette[idx];
  for (int y = 0; y < H; y ++) {
    uint64_t *p = (void *)&screen[y][8];
    for (i = 0; i < nr64; i += 8) {
#define macro(x)  p[i + x] = v
      macro(0); macro(1); macro(2); macro(3);
      macro(4); macro(5); macro(6); macro(7);
    }
  }
#endif

  draw_sync();

}

void xmap_init() {
#ifdef STRETCH
  int w = screen_width();
  int h = screen_height();
  int pad = (w - h) / 2;
  for (int x = pad; x < w - pad; x ++) {
    xmap[x] = (x - pad) * W / h + 0xff;
  }
#endif
}

int main() {
  _ioe_init();

  xmap_init();
  fce_load_rom(rom_mario_nes);
  fce_init();
  fce_run();
  return 1;
}

