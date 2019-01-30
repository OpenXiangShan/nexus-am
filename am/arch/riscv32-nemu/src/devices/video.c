#include <am.h>
#include <riscv32.h>
#include <amdev.h>
#include <klib.h>

#define SCREEN_MMIO 0x4100
static int W, H;
static uint32_t* const fb = (uint32_t *)MMIO_OFFSET(0x40000);

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      info->width = W;
      info->height = H;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;
      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      int len = sizeof(uint32_t) * ( (x + w >= W) ? W - x : w );
      uint32_t *p_fb = &fb[y * W + x];
      int j;

      for (j = 0; j < h; j ++) {
        if (y + j < H) { memcpy(p_fb, pixels, len); }
        else { break; }
        p_fb += W;
        pixels += w;
      }

      if (ctl->sync) {
        // do nothing, hardware syncs.
      }
      return size;
    }
  }
  return 0;
}

void vga_init() {
  uint32_t data = inl(SCREEN_MMIO);
  W = data >> 16;
  H = data & 0xffff;
}
