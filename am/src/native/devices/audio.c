#include <am.h>
#include <amdev.h>
#include <SDL2/SDL.h>
#include <klib.h>

#define SBUF_SIZE 65536
static uint8_t sbuf [SBUF_SIZE] = {};
static int head = 0, tail = 0;
static volatile int count = 0;

static void audio_play(void *userdata, uint8_t *stream, int len) {
  int nread = len;
  if (count < len) nread = count;

  if (nread + tail < SBUF_SIZE) {
    memcpy(stream, sbuf + tail, nread);
    tail += nread;
  } else {
    int first_cpy_len = SBUF_SIZE - tail;
    memcpy(stream, sbuf + tail, first_cpy_len);
    memcpy(stream + first_cpy_len, sbuf, nread - first_cpy_len);
    tail = nread - first_cpy_len;
  }
  count -= nread;
  if (len > nread) memset(stream + nread, 0, len - nread);
}

static int audio_write(uint8_t *buf, int len) {
  int free = SBUF_SIZE - count;
  int nwrite = len;
  if (free < len) nwrite = free;

  if (nwrite + head < SBUF_SIZE) {
    memcpy(sbuf + head, buf, nwrite);
    head += nwrite;
  } else {
    int first_cpy_len = SBUF_SIZE - head;
    memcpy(sbuf + head, buf, first_cpy_len);
    memcpy(sbuf, buf + first_cpy_len, nwrite - first_cpy_len);
    head = nwrite - first_cpy_len;
  }
  count += nwrite;
  return nwrite;
}

void __am_audio_init() {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  SDL_AudioSpec s = {};
  s.freq = 44100;
  s.format = AUDIO_S16SYS;
  s.channels = 1;
  s.samples = 512;
  s.callback = audio_play;
  s.userdata = NULL;

  SDL_OpenAudio(&s, 0);
  SDL_PauseAudio(0);
}

size_t __am_audio_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_AUDIO_SBCTRL: {
      _DEV_AUDIO_SBCTRL_t *ctl = (_DEV_AUDIO_SBCTRL_t *)buf;
      if (ctl->wait) while (count > 0);
      ctl->len = audio_write(ctl->stream, ctl->len);
      return size;
    }
  }
  return 0;
}
