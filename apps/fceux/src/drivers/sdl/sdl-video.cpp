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
/// \brief Handles the graphical game display for the SDL implementation.

#include "sdl.h"
#include "../common/vidblit.h"
#include "../../fceu.h"
#include "../../version.h"
#include "../../video.h"

#include "../../utils/memory.h"

#include "sdl-icon.h"
#include "dface.h"

#include "sdl-video.h"

// STATIC GLOBALS
static SDL_Surface *s_screen;

static SDL_Surface *s_BlitBuf; // Buffer when using hardware-accelerated blits.
static SDL_Surface *s_IconSurface = NULL;

static int s_curbpp;
static int s_srendline, s_erendline;
static int s_tlines;
static int s_inited;

#define s_clipSides 0
static int s_nativeWidth = -1;
static int s_nativeHeight = -1;

#define NWIDTH	(256 - (s_clipSides ? 16 : 0))
#define NOFFSET	(s_clipSides ? 8 : 0)

static int s_paletterefresh;

extern bool MaxSpeed;
 
int
KillVideo()
{
	// if the IconSurface has been initialized, destroy it
	if(s_IconSurface) {
		SDL_FreeSurface(s_IconSurface);
		s_IconSurface=0;
	}

	// return failure if the video system was not initialized
	if(s_inited == 0)
		return -1;
    
	// if the rest of the system has been initialized, shut it down
		// shut down the system that converts from 8 to 16/32 bpp
		if(s_curbpp > 8)
			KillBlitToHigh();

	// shut down the SDL video sub-system
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	s_inited = 0;
	return 0;
}


void FCEUD_VideoChanged()
{
  PAL = 0; // NTSC and Dendy
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
int InitVideo(FCEUGI *gi)
{
	// This is a big TODO.  Stubbing this off into its own function,
	// as the SDL surface routines have changed drastically in SDL2
	// TODO - SDL2
}
#else
/**
 * Attempts to initialize the graphical video display.  Returns 0 on
 * success, -1 on failure.
 */
int
InitVideo(FCEUGI *gi)
{
	// XXX soules - const?  is this necessary?
	const SDL_VideoInfo *vinf;
	int error, flags = 0;

	FCEUI_printf("Initializing video...");

	// check the starting, ending, and total scan lines
	FCEUI_GetCurrentVidSystem(&s_srendline, &s_erendline);
	s_tlines = s_erendline - s_srendline + 1;

	// check if we should auto-set x/y resolution

	// initialize the SDL video subsystem if it is not already active
	if(!SDL_WasInit(SDL_INIT_VIDEO)) {
		error = SDL_InitSubSystem(SDL_INIT_VIDEO);
		if(error) {
			FCEUD_PrintError(SDL_GetError());
			return -1;
		}
	}
	s_inited = 1;

	// shows the cursor within the display window
	SDL_ShowCursor(1);

	// determine if we can allocate the display on the video card
	vinf = SDL_GetVideoInfo();
	if(vinf->hw_available) {
		flags |= SDL_HWSURFACE;
	}
    
	// get the monitor's current resolution if we do not already have it
	if(s_nativeWidth < 0) {
		s_nativeWidth = vinf->current_w;
	}
	if(s_nativeHeight < 0) {
		s_nativeHeight = vinf->current_h;
	}

	FCEUI_SetShowFPS(true);
    
  SDL_ShowCursor(1);
    
	// gives the SDL exclusive palette control... ensures the requested colors
	flags |= SDL_HWPALETTE;

  s_screen = SDL_SetVideoMode(NWIDTH, s_tlines, 32, flags);
		if(!s_screen) {
			FCEUD_PrintError(SDL_GetError());
			return -1;
		}

	s_curbpp = s_screen->format->BitsPerPixel;
	if(!s_screen) {
		FCEUD_PrintError(SDL_GetError());
		KillVideo();
		return -1;
	}

	FCEU_printf(" Video Mode: %d x %d x %d bpp %s\n",
				s_screen->w, s_screen->h, s_screen->format->BitsPerPixel,
				"");

	if(s_curbpp != 8 && s_curbpp != 16 && s_curbpp != 24 && s_curbpp != 32) {
		FCEU_printf("  Sorry, %dbpp modes are not supported by FCE Ultra.  Supported bit depths are 8bpp, 16bpp, and 32bpp.\n", s_curbpp);
		KillVideo();
		return -1;
	}

	// if the game being run has a name, set it as the window name
	if(gi)
	{
		if(gi->name) {
			SDL_WM_SetCaption((const char *)gi->name, (const char *)gi->name);
		} else {
			SDL_WM_SetCaption(FCEU_NAME_AND_VERSION,"FCE Ultra");
		}
	}

	// create the surface for displaying graphical messages
#ifdef LSB_FIRST
	s_IconSurface = SDL_CreateRGBSurfaceFrom((void *)fceu_playicon.pixel_data,
											32, 32, 24, 32 * 3,
											0xFF, 0xFF00, 0xFF0000, 0x00);
#else
	s_IconSurface = SDL_CreateRGBSurfaceFrom((void *)fceu_playicon.pixel_data,
											32, 32, 24, 32 * 3,
											0xFF0000, 0xFF00, 0xFF, 0x00);
#endif
	SDL_WM_SetIcon(s_IconSurface,0);
	s_paletterefresh = 1;

	// XXX soules - can't SDL do this for us?
	 // if using more than 8bpp, initialize the conversion routines
	if(s_curbpp > 8) {
	InitBlitToHigh(s_curbpp >> 3,
						s_screen->format->Rmask,
						s_screen->format->Gmask,
						s_screen->format->Bmask,
						0, 0, 0);
	}
	return 0;
}
#endif

static SDL_Color s_psdl[256];

/**
 * Sets the color for a particular index in the palette.
 */
void
FCEUD_SetPalette(uint8 index,
                 uint8 r,
                 uint8 g,
                 uint8 b)
{
	s_psdl[index].r = r;
	s_psdl[index].g = g;
	s_psdl[index].b = b;

	s_paletterefresh = 1;
}

/**
 * Pushes the given buffer of bits to the screen.
 */
void
BlitScreen(uint8 *XBuf)
{
	SDL_Surface *TmpScreen;
	uint8 *dest;
	int xo = 0, yo = 0;

	if(!s_screen) {
		return;
	}

	// refresh the palette if required
	if(s_paletterefresh) {
    SetPaletteBlitToHigh((uint8*)s_psdl);
		s_paletterefresh = 0;
	}

	// XXX soules - not entirely sure why this is being done yet
	XBuf += s_srendline * 256;

	if(s_BlitBuf) {
		TmpScreen = s_BlitBuf;
	} else {
		TmpScreen = s_screen;
	}

	// lock the display, if necessary
	if(SDL_MUSTLOCK(TmpScreen)) {
		if(SDL_LockSurface(TmpScreen) < 0) {
			return;
		}
	}

	dest = (uint8*)TmpScreen->pixels;

	// XXX soules - again, I'm surprised SDL can't handle this
	// perform the blit, converting bpp if necessary
  Blit8ToHigh(XBuf + NOFFSET, dest, NWIDTH, s_tlines, TmpScreen->pitch, 1, 1);

	// unlock the display, if necessary
	if(SDL_MUSTLOCK(TmpScreen)) {
		SDL_UnlockSurface(TmpScreen);
	}

	int scrw = NWIDTH;

	 // if we have a hardware video buffer, do a fast video->video copy
	if(s_BlitBuf) {
		SDL_Rect srect;
		SDL_Rect drect;

		srect.x = 0;
		srect.y = 0;
		srect.w = scrw;
		srect.h = s_tlines;

		drect.x = 0;
		drect.y = 0;
		drect.w = (Uint16)(scrw);
		drect.h = (Uint16)(s_tlines);

		SDL_BlitSurface(s_BlitBuf, &srect, s_screen, &drect);
	}

	 // ensure that the display is updated
#if SDL_VERSION_ATLEAST(2, 0, 0)
	//TODO - SDL2
#else
	SDL_UpdateRect(s_screen, xo, yo, scrw, s_tlines);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 0)
	// TODO
#else
    // have to flip the displayed buffer in the case of double buffering
	if(s_screen->flags & SDL_DOUBLEBUF) {
		SDL_Flip(s_screen);
	}
#endif
}
