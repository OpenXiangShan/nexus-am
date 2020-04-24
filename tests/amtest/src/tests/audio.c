#include <amtest.h>
#include <amdev.h>

void audio_test() {
  _DEV_AUDIO_INIT_t init;
  init.freq = 8000;
  init.channels = 1;
  init.samples = 1024;
  init.bufsize = 8192;
  _io_write(_DEV_AUDIO, _DEVREG_AUDIO_INIT, &init, sizeof(init));

  extern uint8_t audio_payload, audio_payload_end;
  uint32_t audio_len = &audio_payload_end - &audio_payload;
  _DEV_AUDIO_SBCTRL_t ctl;
  int nplay = 0;
  ctl.stream = &audio_payload;
  ctl.wait = 1;
  while (nplay < audio_len) {
    ctl.len = (audio_len - nplay > 4096 ? 4096 : audio_len - nplay);
    _io_write(_DEV_AUDIO, _DEVREG_AUDIO_SBCTRL, &ctl, sizeof(ctl));
    ctl.stream += ctl.len;
    nplay += ctl.len;
    printf("Already play %d/%d bytes of data\n", nplay, audio_len);
  }

  // wait until the audio finishes
  _DEV_AUDIO_SBSTAT_t stat;
  do {
    _io_read(_DEV_AUDIO, _DEVREG_AUDIO_SBSTAT, &stat, sizeof(stat));
  } while (stat.count > 0);
}
