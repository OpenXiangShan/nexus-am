#include <am.h>
#include <amdev.h>
#include <ndl.h>

#define W 400
#define H 300

void video_init() {
  NDL_OpenDisplay(W, H);
}

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
      NDL_DrawRect(pixels, x, y, w, h);
      if (ctl->sync) {
        NDL_Render();
      }
      return size;
    }
  }
  return 0;
}
