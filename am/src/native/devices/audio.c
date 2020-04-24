#include <am.h>
#include <amdev.h>
#include <SDL2/SDL.h>
#include <klib.h>

#define SBUF_SIZE_MAX 65536
static uint8_t sbuf [SBUF_SIZE_MAX] = {};
static int sbuf_size = 0;
static int head = 0, tail = 0;
static volatile int count = 0;

static void audio_play(void *userdata, uint8_t *stream, int len) {
  int nread = len;
  if (count < len) nread = count;

  if (nread + tail < sbuf_size) {
    memcpy(stream, sbuf + tail, nread);
    tail += nread;
  } else {
    int first_cpy_len = sbuf_size - tail;
    memcpy(stream, sbuf + tail, first_cpy_len);
    memcpy(stream + first_cpy_len, sbuf, nread - first_cpy_len);
    tail = nread - first_cpy_len;
  }
  count -= nread;
  if (len > nread) memset(stream + nread, 0, len - nread);
}

static int audio_write(uint8_t *buf, int len) {
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
  return nwrite;
}

void __am_audio_init() {
}

size_t __am_audio_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_AUDIO_INIT: {
      _DEV_AUDIO_INIT_t *init = (_DEV_AUDIO_INIT_t *)buf;
      SDL_AudioSpec s = {};
      s.freq = init->freq;
      s.format = AUDIO_S16SYS;
      s.channels = init->channels;
      s.samples = init->samples;
      s.callback = audio_play;
      s.userdata = NULL;
      sbuf_size = init->bufsize;
      assert(sbuf_size <= SBUF_SIZE_MAX);

      head = tail = 0;
      count = 0;
      SDL_InitSubSystem(SDL_INIT_AUDIO);
      SDL_OpenAudio(&s, NULL);
      SDL_PauseAudio(0);
      return size;
    }
    case _DEVREG_AUDIO_SBCTRL: {
      _DEV_AUDIO_SBCTRL_t *ctl = (_DEV_AUDIO_SBCTRL_t *)buf;
      if (ctl->wait) {
        assert(ctl->len <= sbuf_size);
        while (sbuf_size - count < ctl->len);
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
      stat->count = count;
      stat->bufsize = sbuf_size;
      return size;
    }
  }
  return 0;
}
