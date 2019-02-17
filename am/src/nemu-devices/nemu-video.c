#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <nemu.h>

static int W, H;
static uint32_t* const fb = (uint32_t *)FB_ADDR;

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _DEV_VIDEO_INFO_t *info = (_DEV_VIDEO_INFO_t *)buf;
      info->width = W;
      info->height = H;
      return sizeof(_DEV_VIDEO_INFO_t);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _DEV_VIDEO_FBCTL_t *ctl = (_DEV_VIDEO_FBCTL_t *)buf;
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
        outl(SYNC_ADDR, 0);
      }
      return size;
    }
  }
  return 0;
}

void vga_init() {
  uint32_t data = inl(SCREEN_ADDR);
  W = data >> 16;
  H = data & 0xffff;
}
