#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <nemu.h>

static uint8_t* const sbuf = (uint8_t *)AUDIO_SBUF_ADDR;
static int sbuf_size = 0;
static int head = 0;

void __am_audio_init() {
}

static int audio_write(uint8_t *buf, int len) {
  uint32_t count = inl(AUDIO_COUNT_ADDR);
  int free = sbuf_size - count;
  int nwrite = len;
  if (free < len) nwrite = free;

  if (nwrite + head < sbuf_size) {
    memcpy(sbuf + head, buf, nwrite);
    head += nwrite;
  } else {
    int first_cpy_len = sbuf_size - head;
    memcpy(sbuf + head, buf, first_cpy_len);
    memcpy(sbuf, buf + first_cpy_len, nwrite - first_cpy_len);
    head = nwrite - first_cpy_len;
  }
  count += nwrite;
  outl(AUDIO_COUNT_ADDR, count);
  return nwrite;
}

size_t __am_audio_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_AUDIO_INIT: {
      _DEV_AUDIO_INIT_t *init = (_DEV_AUDIO_INIT_t *)buf;
      outl(AUDIO_FREQ_ADDR, init->freq);
      outl(AUDIO_CHANNELS_ADDR, init->channels);
      outl(AUDIO_SAMPLES_ADDR, init->samples);
      outl(AUDIO_SBUF_SIZE_ADDR, init->bufsize);
      outl(AUDIO_INIT_ADDR, 1);
      sbuf_size = init->bufsize;

      head = 0;
      return size;
    }
    case _DEVREG_AUDIO_SBCTRL: {
      _DEV_AUDIO_SBCTRL_t *ctl = (_DEV_AUDIO_SBCTRL_t *)buf;
      if (ctl->wait) {
        assert(ctl->len <= sbuf_size);
        while (sbuf_size - inl(AUDIO_COUNT_ADDR) < ctl->len);
      }
      ctl->len = audio_write(ctl->stream, ctl->len);
      return size;
    }
  }
  return 0;
}

size_t __am_audio_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_AUDIO_SBSTAT: {
      _DEV_AUDIO_SBSTAT_t *stat = (_DEV_AUDIO_SBSTAT_t *)buf;
      stat->count = inl(AUDIO_COUNT_ADDR);
      stat->bufsize = sbuf_size;
      return size;
    }
  }
  return 0;
}
