/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/// \file
/// \brief Handles sound emulation using the SDL.

#include "sdl.h"
#include <amdev.h>

//#include "../common/configSys.h"
#include "../../utils/memory.h"

//extern Config *g_config;

static volatile int *s_Buffer = 0;
static unsigned int s_BufferSize;
static unsigned int s_BufferRead;
static unsigned int s_BufferWrite;
static volatile unsigned int s_BufferIn;

//static int s_mute = 0;


/**
 * Callback from the SDL to get and play audio data.
 */
#if 0
  static void
fillaudio(void *udata,
    uint8 *stream,
    int len)
{
  int16 *tmps = (int16*)stream;
  len >>= 1;
  while(len) {
    int16 sample = 0;
    if(s_BufferIn) {
      sample = s_Buffer[s_BufferRead];
      s_BufferRead = (s_BufferRead + 1) % s_BufferSize;
      s_BufferIn--;
    } else {
      sample = 0;
    }

    *tmps = sample;
    tmps++;
    len--;
  }
}
#endif

/**
 * Initialize the audio subsystem.
 */
  int
InitSound()
{
  int soundrate = 44100;
  int soundbufsize = 128;
  int soundvolume = 150;
  int soundtrianglevolume = 256;
  int soundsquare1volume = 256;
  int soundsquare2volume = 256;
  int soundnoisevolume = 256;
  int soundpcmvolume = 256;
  int soundq = 1;

#if 0
  spec.freq = soundrate;
  spec.format = AUDIO_S16SYS;
  spec.channels = 1;
  spec.samples = 512;
  spec.callback = fillaudio;
  spec.userdata = 0;
#endif

  s_BufferSize = soundbufsize * soundrate / 1000;

  // For safety, set a bare minimum:
  //if (s_BufferSize < spec.samples * 2)
  //s_BufferSize = spec.samples * 2;

  s_Buffer = (int *)FCEU_dmalloc(sizeof(int) * s_BufferSize);
  if (!s_Buffer)
    return 0;
  s_BufferRead = s_BufferWrite = s_BufferIn = 0;

#if 0
  if(SDL_OpenAudio(&spec, 0) < 0)
  {
    puts(SDL_GetError());
    KillSound();
    return 0;
  }
  SDL_PauseAudio(0);
#endif

  FCEUI_SetSoundVolume(soundvolume);
  FCEUI_SetSoundQuality(soundq);
  FCEUI_Sound(soundrate);
  FCEUI_SetTriangleVolume(soundtrianglevolume);
  FCEUI_SetSquare1Volume(soundsquare1volume);
  FCEUI_SetSquare2Volume(soundsquare2volume);
  FCEUI_SetNoiseVolume(soundnoisevolume);
  FCEUI_SetPCMVolume(soundpcmvolume);
  return 1;
}


/**
 * Returns the size of the audio buffer.
 */
  uint32
GetMaxSound(void)
{
  return(s_BufferSize);
}

/**
 * Returns the amount of free space in the audio buffer.
 */
  uint32
GetWriteSound(void)
{
  return(s_BufferSize - s_BufferIn);
}

/**
 * Send a sound clip to the audio subsystem.
 */
  void
WriteSound(int32 *buf,
    int Count)
{
  extern int EmulationPaused;
  if (EmulationPaused == 0) {
    _DEV_AUDIO_SBCTRL_t audio;
    audio.stream = (uint8_t *)buf;
    audio.wait = 1;
    uint32_t audio_len = Count * sizeof(buf[0]);
    uint32_t nplay = 0;
    while (nplay < audio_len) {
      audio.len = (audio_len - nplay > 4096 ? 4096 : audio_len - nplay);
      _io_write(_DEV_AUDIO, _DEVREG_AUDIO_SBCTRL, &audio, sizeof(audio));
      audio.stream += audio.len;
      nplay += audio.len;
    }
  }

#if 0
  while(Count)
  {
    while(s_BufferIn == s_BufferSize)
    {
      SDL_Delay(1);
    }

    s_Buffer[s_BufferWrite] = *buf;
    Count--;
    s_BufferWrite = (s_BufferWrite + 1) % s_BufferSize;

    SDL_LockAudio();
    s_BufferIn++;
    SDL_UnlockAudio();

    buf++;
  }
#endif
}

/**
 * Pause (1) or unpause (0) the audio output.
 */
  void
SilenceSound(int n)
{
  //SDL_PauseAudio(n);
}

/**
 * Shut down the audio subsystem.
 */
  int
KillSound(void)
{
  FCEUI_Sound(0);
  //SDL_CloseAudio();
  //SDL_QuitSubSystem(SDL_INIT_AUDIO);
  if(s_Buffer) {
    free((void *)s_Buffer);
    s_Buffer = 0;
  }
  return 0;
}


/**
 * Adjust the volume either down (-1), up (1), or to the default (0).
 * Unmutes if mute was active before.
 */
  void
FCEUD_SoundVolumeAdjust(int n)
{
#if 0
  int soundvolume;
  g_config->getOption("SDL.Sound.Volume", &soundvolume);

  switch(n) {
    case -1:
      soundvolume -= 10;
      if(soundvolume < 0) {
        soundvolume = 0;
      }
      break;
    case 0:
      soundvolume = 100;
      break;
    case 1:
      soundvolume += 10;
      if(soundvolume > 150) {
        soundvolume = 150;
      }
      break;
  }

  s_mute = 0;
  FCEUI_SetSoundVolume(soundvolume);
  g_config->setOption("SDL.Sound.Volume", soundvolume);

  printf("Sound volume %d.\n", soundvolume);
#endif
}

/**
 * Toggles the sound on or off.
 */
  void
FCEUD_SoundToggle(void)
{
#if 0
  if(s_mute) {
    int soundvolume;
    g_config->getOption("SDL.SoundVolume", &soundvolume);

    s_mute = 0;
    FCEUI_SetSoundVolume(soundvolume);
    FCEU_DispMessage("Sound mute off.");
  } else {
    s_mute = 1;
    FCEUI_SetSoundVolume(0);
    FCEU_DispMessage("Sound mute on.");
  }
#endif
}
