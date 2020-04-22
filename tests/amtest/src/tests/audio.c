#include <amtest.h>
#include <amdev.h>

void audio_test() {
  extern uint8_t audio_payload, audio_payload_end;
  _DEV_AUDIO_SBCTRL_t audio;
  int nplay = 0;
  audio.stream = &audio_payload;
  audio.wait = 1;
  uint32_t audio_len = &audio_payload_end - &audio_payload;
  while (nplay < audio_len) {
    audio.len = (audio_len - nplay > 4096 ? 4096 : audio_len - nplay);
    _io_write(_DEV_AUDIO, _DEVREG_AUDIO_SBCTRL, &audio, sizeof(audio));
    audio.stream += audio.len;
    nplay += audio.len;
    printf("Already play %d/%d bytes of data\n", nplay, audio_len);
  }
}
