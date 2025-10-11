#include <klib.h>
#include <csr.h>

unsigned short imgY_org[256];
unsigned short imgX_ref[256][256];

#define NTIMES 100

int byte_abs_raw[512];
int* byte_abs;
int  block_sad[16][9];

short *FastLine16Y_11 (short *Pic, int y, int x, int height, int width)
{
  return &Pic [y*width+x];
}

void init_byte_abs()
{
  int byte_abs_range = 512;
  byte_abs = (int*)((uint64_t)byte_abs_raw + sizeof(byte_abs_raw) / 2);
  for (int i = 0; i < 256; i++) {
    byte_abs[i] = i;
    byte_abs[-i] = i;
  }
}

void SetupFastFullPelSearch ()  // <--  reference frame parameter, list0 or 1
{
  int     x, y, pos, bindex, blky;
  int     LineSadBlk0, LineSadBlk1, LineSadBlk2, LineSadBlk3;
  short*  orgptr;
  short*  refptr;
  int abs_x = 0;
  int abs_y = 0;

  int     search_range  = 16;
  int     max_pos       = 9;

  int     list_offset   = 0;

  int     apply_weights = 0;

  //===== loop over search range (spiral search): get blockwise SAD =====
  for (pos = 0; pos < max_pos; pos++)
  {
    orgptr = (short*)imgY_org;
    bindex = 0;
    for (blky = 0; blky < 4; blky++)
    {
      LineSadBlk0 = LineSadBlk1 = LineSadBlk2 = LineSadBlk3 = 0;
      for (y = 0; y < 4; y++)
      {
        refptr = FastLine16Y_11 ((short*)imgX_ref, abs_y++, abs_x, 256, 256);

        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
      }
      block_sad[bindex++][pos] = LineSadBlk0;
      block_sad[bindex++][pos] = LineSadBlk1;
      block_sad[bindex++][pos] = LineSadBlk2;
      block_sad[bindex++][pos] = LineSadBlk3;
    }
  }
}

int main() {

  init_byte_abs();

  // warmup
  SetupFastFullPelSearch();

  uint64_t cycle_count = 0;
  uint64_t start_time = csr_read(CSR_MCYCLE);
  uint64_t start_inst = csr_read(CSR_MINSTRET);
  for (int i = 0; i < NTIMES; i++) {
    SetupFastFullPelSearch();
  }
  uint64_t end_time = csr_read(CSR_MCYCLE);
  uint64_t end_inst = csr_read(CSR_MINSTRET);
  cycle_count = end_time - start_time;
  printf("SetupFastFullPelSearch took %lu cycles\n", cycle_count);
  printf("SetupFastFullPelSearch took %lu instructions\n", end_inst - start_inst);
  printf("IPC: %f\n", (double)(end_inst - start_inst) / (double)cycle_count);
  printf("bandwidth: %f SetupFastFullPelSearch/Cycle\n", NTIMES / (double)cycle_count);

  int tmp = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 9; j++) {
      tmp += block_sad[i][j];
    }
  }

  printf("tmp: %d\n", tmp);

  return 0;
}
