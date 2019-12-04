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

#include "main.h"
#include "dface.h"
#include "input.h"


#include "sdl-video.h"
#include "sdl.h"

#include "../../fceu.h"
#include "../../driver.h"

/** GLOBALS **/
int NoWaiting = 1;
extern bool bindSavestate, frameAdvanceLagSkip, lagCounterDisplay;


/* UsrInputType[] is user-specified.  CurInputType[] is current
        (game loading can override user settings)
*/
static int UsrInputType[NUM_INPUT_DEVICES];
static int CurInputType[NUM_INPUT_DEVICES];
static int cspec = 0;

extern int gametype;

/**
 * Necessary for proper GUI functioning (configuring when a game isn't loaded).
 */
void
InputUserActiveFix ()
{
	int x;
	for (x = 0; x < 3; x++)
	{
		CurInputType[x] = UsrInputType[x];
	}
}

/**
 * Parse game information and configure the input devices accordingly.
 */
void
ParseGIInput (FCEUGI * gi)
{
	gametype = gi->type;

	CurInputType[0] = UsrInputType[0];
	CurInputType[1] = UsrInputType[1];
	CurInputType[2] = UsrInputType[2];

	if (gi->input[0] >= 0)
	{
		CurInputType[0] = gi->input[0];
	}
	if (gi->input[1] >= 0)
	{
		CurInputType[1] = gi->input[1];
	}
	if (gi->inputfc >= 0)
	{
		CurInputType[2] = gi->inputfc;
	}
	cspec = gi->cspecial;
}


static void UpdateGamepad (void);

static uint32 JSreturn = 0;

#include "keyscan.h"
static uint8 *g_keyState = 0;

static uint8 keyonce[MKK_COUNT];
#define KEY(__a) g_keyState[MKK(__a)]

static int
_keyonly (int a)
{
	// check for valid key
	if (a > SDLK_LAST + 1 || a < 0)
		return 0;
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (g_keyState[SDL_GetScancodeFromKey (a)])
#else
	if (g_keyState[a])
#endif
	{
		if (!keyonce[a])
		{
			keyonce[a] = 1;
			return 1;
		}
	} 
	else {
		keyonce[a] = 0;
	}
	return 0;
}

#define keyonly(__a) _keyonly(MKK(__a))

/**
* Hook for transformer board
*/
unsigned int *GetKeyboard(void)                                                     
{
	int size = 256;
#if SDL_VERSION_ATLEAST(2, 0, 0)
	Uint8* keystate = (Uint8*)SDL_GetKeyboardState(&size);
#else
	Uint8* keystate = SDL_GetKeyState(&size);
#endif
	return (unsigned int*)(keystate);
}

/**
 * Parse keyboard commands and execute accordingly.
 */
static void KeyboardCommands ()
{
	// get the keyboard input
#if SDL_VERSION_ATLEAST(1, 3, 0)
	g_keyState = (Uint8*)SDL_GetKeyboardState (NULL);
#else
	g_keyState = SDL_GetKeyState (NULL);
#endif

	// Toggle throttling
	NoWaiting &= ~1;

	{
		static uint8 bbuf[32];
		static int bbuft;
		static int barcoder = 0;

		if (keyonly (H))
			FCEUI_NTSCSELHUE ();
		if (keyonly (T))
			FCEUI_NTSCSELTINT ();

		if ((CurInputType[2] == SIFC_BWORLD) || (cspec == SIS_DATACH))
		{
			if (keyonly (F8))
			{
				barcoder ^= 1;
				if (!barcoder)
				{
          FCEUI_DatachSet (bbuf);
					FCEUI_DispMessage ("Barcode Entered", 0);
				}
				else
				{
					bbuft = 0;
					FCEUI_DispMessage ("Enter Barcode", 0);
				}
			}
		}
		else
		{
			barcoder = 0;
		}

#define SSM(x)                                    \
do {                                              \
	if(barcoder) {                                \
		if(bbuft < 13) {                          \
			bbuf[bbuft++] = '0' + x;              \
			bbuf[bbuft] = 0;                      \
		}                                         \
		FCEUI_DispMessage("Barcode: %s",0, bbuf); \
	}                                             \
} while(0)

		for(int i=0; i<10;i++)
		{
			if (keyonly (i))
				SSM (i);
		}
#undef SSM
	}
}

/**
 * Return the state of the mouse buttons.  Input 'd' is an array of 3
 * integers that store <x, y, button state>.
 */
void				// removed static for a call in lua-engine.cpp
GetMouseData (uint32 (&d)[3])
{
  return;
}

/**
 * Handles outstanding SDL events.
 */
static void
UpdatePhysicalInput ()
{
	SDL_Event event;

	// loop, handling all pending events
	while (SDL_PollEvent (&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			  CloseGame ();
			  puts ("Quit");
			  break;
			default:
				break;
		}
	}
	//SDL_PumpEvents();
}


static int bcpv, bcpj;

/**
 *  Begin configuring the buttons by placing the video and joystick
 *  subsystems into a well-known state.  Button configuration really
 *  needs to be cleaned up after the new config system is in place.
 */
int ButtonConfigBegin ()
{
//dont shut down video subsystem if we are using gtk to prevent the sdl window from becoming detached to GTK window
// prg318 - 10-2-2011
	// XXX soules - why are we doing this right before KillVideo()?
	SDL_QuitSubSystem (SDL_INIT_VIDEO);

  assert(0);
	// shut down the video and joystick subsystems
	bcpv = KillVideo ();
	//SDL_Surface *screen;

	bcpj = KillJoysticks ();

	// reactivate the video subsystem
	if (!SDL_WasInit (SDL_INIT_VIDEO))
	{
		if (!bcpv)
		{
			InitVideo (GameInfo);
		}
		else
		{
			if (SDL_InitSubSystem (SDL_INIT_VIDEO) == -1)
			{
				FCEUD_Message (SDL_GetError ());
				return 0;
			}

			// set the screen and notify the user of button configuration
#if SDL_VERSION_ATLEAST(2, 0, 0)
			// TODO - SDL2
#else
			SDL_SetVideoMode (420, 200, 8, 0);
			SDL_WM_SetCaption ("Button Config", 0);
#endif
		}
	}

	// XXX soules - why did we shut this down?
	// initialize the joystick subsystem
	InitJoysticks ();

	return 1;
}

/**
 * Tests to see if a specified button is currently pressed.
 */
static int
DTestButton (ButtConfig * bc)
{
	int x;

	for (x = 0; x < (int)bc->NumC; x++)
	{
		if (bc->ButtType[x] == BUTTC_KEYBOARD)
		{
#if SDL_VERSION_ATLEAST(2, 0, 0)
			if (g_keyState[SDL_GetScancodeFromKey (bc->ButtonNum[x])])
			{
#else
			if (g_keyState[bc->ButtonNum[x]])
			{
#endif
				return 1;
			}
		}
    }
  return 0;
}


#define MK(x)       {{BUTTC_KEYBOARD},{0},{MKK(x)},1}
#define MK2(x1,x2)  {{BUTTC_KEYBOARD},{0},{MKK(x1),MKK(x2)},2}
#define MKZ()       {{0},{0},{0},0}
#define GPZ()       {MKZ(), MKZ(), MKZ(), MKZ()}

ButtConfig GamePadConfig[4][10] = {
#if SDL_VERSION_ATLEAST(2, 0, 0)
/* Gamepad 1 */
	{MK (KP_3), MK (KP_2), MK (SLASH), MK (ENTER),
	MK (W), MK (Z), MK (A), MK (S), MKZ (), MKZ ()},
#else
	/* Gamepad 1 */
	{MK (KP3), MK (KP2), MK (SLASH), MK (ENTER),
	MK (W), MK (Z), MK (A), MK (S), MKZ (), MKZ ()},
#endif

	/* Gamepad 2 */
	GPZ (),

	/* Gamepad 3 */
	GPZ (),

	/* Gamepad 4 */
	GPZ ()
};

/**
 * Update the status of the gamepad input devices.
 */
static void
UpdateGamepad(void)
{
	static int rapid = 0;
	uint32 JS = 0;
	int x;
	int wg;

	rapid ^= 1;

	// go through each of the four game pads
	for (wg = 0; wg < 4; wg++)
	{
		// a, b, select, start, up, down, left, right
		for (x = 0; x < 8; x++)
		{
			if (DTestButton (&GamePadConfig[wg][x]))
			{
				JS |= (1 << x) << (wg << 3);
			}
		}

		// rapid-fire a, rapid-fire b
		if (rapid)
		{
			for (x = 0; x < 2; x++)
			{
				if (DTestButton (&GamePadConfig[wg][8 + x]))
				{
					JS |= (1 << x) << (wg << 3);
				}
			}
		}
	}

	JSreturn = JS;
}

/**
 * Update all of the input devices required for the active game.
 */
void FCEUD_UpdateInput ()
{
	int x;
	int t = 0;

	UpdatePhysicalInput ();
	KeyboardCommands ();

	for (x = 0; x < 2; x++)
	{
		switch (CurInputType[x])
		{
			case SI_GAMEPAD:
				t |= 1;
				break;
		}
	}

	if (t & 1)
	{
		UpdateGamepad ();
	}
}

void FCEUD_SetInput (bool fourscore, bool microphone, ESI port0, ESI port1,
		ESIFC fcexp)
{
	eoptions &= ~EO_FOURSCORE;
	if (fourscore)
	{				// Four Score emulation, only support gamepads, nothing else
		eoptions |= EO_FOURSCORE;
		CurInputType[0] = SI_GAMEPAD;	// Controllers 1 and 3
		CurInputType[1] = SI_GAMEPAD;	// Controllers 2 and 4
		CurInputType[2] = SIFC_NONE;	// No extension
	}
	else
	{	
		// no Four Core emulation, check the config/movie file for controller types
		CurInputType[0] = port0;
		CurInputType[1] = port1;
		CurInputType[2] = fcexp;
	}

	InitInputInterface ();
}

/**
 * Initialize the input device interface between the emulation and the driver.
 */
void InitInputInterface ()
{
	void *InputDPtr;

	int x;
	int attrib;

	for (x = 0; x < 2; x++)
	{
		attrib = 0;
		InputDPtr = 0;

		switch (CurInputType[x])
		{
			case SI_GAMEPAD:
				InputDPtr = &JSreturn;
				break;
		}
		FCEUI_SetInput (x, (ESI) CurInputType[x], InputDPtr, attrib);
	}

	attrib = 0;
	InputDPtr = 0;

	FCEUI_SetInputFC ((ESIFC) CurInputType[2], InputDPtr, attrib);
	FCEUI_SetInputFourscore ((eoptions & EO_FOURSCORE) != 0);
}

/**
 * Update the button configuration for a device, specified by a text string.
 */
void InputCfg (const std::string & text)
{
  assert(0);
}


/**
 * Hack to map the new configuration onto the existing button
 * configuration management.  Will probably want to change this in the
 * future - soules.
 */
	void UpdateInput ()
{
	for (unsigned int i = 0; i < 3; i++) {
    UsrInputType[i] = (i < 2) ? (int) SI_GAMEPAD : (int) SIFC_NONE;
	}

	// gamepad 0 - 3
	for (unsigned int i = 0; i < GAMEPAD_NUM_DEVICES; i++) {
    int type = (i == 0 ? BUTTC_KEYBOARD : 0);

		for (unsigned int j = 0; j < GAMEPAD_NUM_BUTTONS; j++) {
			GamePadConfig[i][j].ButtType[0] = type;
			GamePadConfig[i][j].DeviceNum[0] = 0;
			GamePadConfig[i][j].ButtonNum[0] = DefaultGamePad[i][j];
			GamePadConfig[i][j].NumC = 1;
		}
	}
}

// Definitions from main.h:
// GamePad defaults
const char *GamePadNames[GAMEPAD_NUM_BUTTONS] = { "A", "B", "Select", "Start",
	"Up", "Down", "Left", "Right", "TurboA", "TurboB"
};
const char *DefaultGamePadDevice[GAMEPAD_NUM_DEVICES] =
{ "Keyboard", "None", "None", "None" };
const int DefaultGamePad[GAMEPAD_NUM_DEVICES][GAMEPAD_NUM_BUTTONS] =
{ {SDLK_J, SDLK_K, SDLK_U, SDLK_I,
	SDLK_W, SDLK_S, SDLK_A, SDLK_D, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};
