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

#include "types.h"
#include "video.h"
#include "fceu.h"
#include "file.h"
#include "utils/memory.h"
#include "state.h"
#include "palette.h"
#include "input.h"
#include "vsuni.h"
#include "drawing.h"
#include "driver.h"
#include "drivers/common/vidblit.h"

#include <stdint.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

//XBuf:
//0-63 is reserved for 7 special colours used by FCEUX (overlay, etc.)
//64-127 is the most-used emphasis setting per frame
//128-195 is the palette with no emphasis
//196-255 is the palette with all emphasis bits on
u8 *XBuf=NULL; //used for current display
u8 *XBackBuf=NULL; //ppu output is stashed here before drawing happens
u8 *XDBuf=NULL; //corresponding to XBuf but with deemph bits
u8 *XDBackBuf=NULL; //corresponding to XBackBuf but with deemph bits
int ClipSidesOffset=0;	//Used to move displayed messages when Clips left and right sides is checked
static u8 *xbsave=NULL;

GUIMESSAGE guiMessage;
GUIMESSAGE subtitleMessage;

//for input display
extern int input_display;
extern uint32 cur_input_display;

bool oldInputDisplay = false;

unsigned int lastu = 0;

std::string AsSnapshotName ="";			//adelikat:this will set the snapshot name when for s savesnapshot as function

void FCEUI_SetSnapshotAsName(std::string name) { AsSnapshotName = name; }
std::string FCEUI_GetSnapshotAsName() { return AsSnapshotName; }

void FCEU_KillVirtualVideo(void)
{
	//mbg merge TODO 7/17/06 temporarily removed
	//if(xbsave)
	//{
	// free(xbsave);
	// xbsave=0;
	//}
	//if(XBuf)
	//{
	//UnmapViewOfFile(XBuf);
	//CloseHandle(mapXBuf);
	//mapXBuf=NULL;
	//}
	//if(XBackBuf)
	//{
	// free(XBackBuf);
	// XBackBuf=0;
	//}
}

/**
* Return: Flag that indicates whether the function was succesful or not.
*
* TODO: This function is Windows-only. It should probably be moved.
**/
int FCEU_InitVirtualVideo(void)
{
	//Some driver code may allocate XBuf externally.
	//256 bytes per scanline, * 240 scanline maximum, +16 for alignment,
	if(XBuf)
		return 1;
	
	XBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	XBackBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	XDBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	XDBackBuf = (u8*)FCEU_malloc(256 * 256 + 16);
	if(!XBuf || !XBackBuf || !XDBuf || !XDBackBuf)
	{
		return 0;
	}

	xbsave = XBuf;

	if( sizeof(uint8*) == 4 )
	{
		uintptr_t m = (uintptr_t)XBuf;
		m = ( 8 - m) & 7;
		XBuf+=m;
	}

	memset(XBuf,128,256*256);
	memset(XBackBuf,128,256*256);
	memset(XBuf,128,256*256);
	memset(XBackBuf,128,256*256);

	return 1;
}

#ifdef FRAMESKIP
void FCEU_PutImageDummy(void)
{
	ShowFPS();
	if(GameInfo->type!=GIT_NSF)
	{
		FCEU_DrawNTSCControlBars(XBuf);
		//FCEU_DrawSaveStates(XBuf);
		//FCEU_DrawMovies(XBuf);
	}
	if(guiMessage.howlong) guiMessage.howlong--; /* DrawMessage() */
}
#endif

static int dosnapsave=0;
void FCEUI_SaveSnapshot(void)
{
	dosnapsave=1;
}

void FCEUI_SaveSnapshotAs(void)
{
	dosnapsave=2;
}

static void ReallySnap(void)
{
	int x=SaveSnapshot();
	if(!x)
		FCEU_DispMessage("Error saving screen snapshot.",0);
	else
		FCEU_DispMessage("Screen snapshot %d saved.",0,x-1);
}

void FCEU_PutImage(void)
{
	if(dosnapsave==2)	//Save screenshot as, currently only flagged & run by the Win32 build. //TODO SDL: implement this?
	{
		char nameo[512];
		strcpy(nameo,FCEUI_GetSnapshotAsName().c_str());
		if (nameo[0])
		{
			SaveSnapshot(nameo);
			FCEU_DispMessage("Snapshot Saved.",0);
		}
		dosnapsave=0;
	}
	if(GameInfo->type==GIT_NSF)
	{
		//DrawNSF(XBuf);
    assert(0);

		//Save snapshot after NSF screen is drawn.  Why would we want to do it before?
		if(dosnapsave==1)
		{
			ReallySnap();
			dosnapsave=0;
		}
	}
	else
	{
		//Save backbuffer before overlay stuff is written.
		if(!FCEUI_EmulationPaused())
			memcpy(XBackBuf, XBuf, 256*256);

		//Some messages need to be displayed before the avi is dumped
		DrawMessage(true);

		//Save snapshot
		if(dosnapsave==1)
		{
			ReallySnap();
			dosnapsave=0;
		}

		if (!FCEUI_AviEnableHUDrecording()) snapAVI();

		if(GameInfo->type==GIT_VSUNI)
			FCEU_VSUniDraw(XBuf);

		//FCEU_DrawSaveStates(XBuf);
		//FCEU_DrawMovies(XBuf);
		//FCEU_DrawLagCounter(XBuf);
		FCEU_DrawNTSCControlBars(XBuf);
		FCEU_DrawRecordingStatus(XBuf);
		ShowFPS();
	}

	if(FCEUD_ShouldDrawInputAids())
		FCEU_DrawInput(XBuf);

	if (FCEUI_AviEnableHUDrecording())
	{
		if (FCEUI_AviDisableMovieMessages())
		{
			snapAVI();
			DrawMessage(false);
		} else
		{
			DrawMessage(false);
			snapAVI();
		}
	} else DrawMessage(false);

}
void snapAVI()
{
	//Update AVI
	if(!FCEUI_EmulationPaused())
		FCEUI_AviVideoUpdate(XBuf);
}

void FCEU_DispMessageOnMovie(const char *format, ...)
{
	va_list ap;

	va_start(ap,format);
	vsnprintf(guiMessage.errmsg,sizeof(guiMessage.errmsg),format,ap);
	va_end(ap);

	guiMessage.howlong = 180;
	guiMessage.isMovieMessage = true;
	guiMessage.linesFromBottom = 0;

	if (FCEUI_AviIsRecording() && FCEUI_AviDisableMovieMessages())
		guiMessage.howlong = 0;
}

void FCEU_DispMessage(const char *format, int disppos=0, ...)
{
	va_list ap;

	va_start(ap,disppos);
	vsnprintf(guiMessage.errmsg,sizeof(guiMessage.errmsg),format,ap);
	va_end(ap);
	// also log messages
	char temp[2048];
	va_start(ap,disppos);
	vsnprintf(temp,sizeof(temp),format,ap);
	va_end(ap);
	strcat(temp, "\n");
	FCEU_printf(temp);

	guiMessage.howlong = 180;
	guiMessage.isMovieMessage = false;

	guiMessage.linesFromBottom = disppos;

	//adelikat: Pretty sure this code fails, Movie playback stopped is done with FCEU_DispMessageOnMovie()
}

void FCEU_ResetMessages()
{
	guiMessage.howlong = 0;
	guiMessage.isMovieMessage = false;
	guiMessage.linesFromBottom = 0;
}


uint32 GetScreenPixel(int x, int y, bool usebackup) {

	uint8 r,g,b;

	if (((x < 0) || (x > 255)) || ((y < 0) || (y > 255)))
		return -1;

	if (usebackup)
		FCEUD_GetPalette(XBackBuf[(y*256)+x],&r,&g,&b);
	else
		FCEUD_GetPalette(XBuf[(y*256)+x],&r,&g,&b);


	return ((int) (r) << 16) | ((int) (g) << 8) | (int) (b);
}

int GetScreenPixelPalette(int x, int y, bool usebackup) {

	if (((x < 0) || (x > 255)) || ((y < 0) || (y > 255)))
		return -1;

	if (usebackup)
		return XBackBuf[(y*256)+x] & 0x3f;
	else
		return XBuf[(y*256)+x] & 0x3f;

}

int SaveSnapshot(void)
{
	return(0);
}

//overloaded SaveSnapshot for "Savesnapshot As" function
int SaveSnapshot(char fileName[512])
{
	return 0;
}
// called when another ROM is opened
void ResetScreenshotsCounter()
{
	lastu = 0;
}

uint64 FCEUD_GetTime(void);
uint64 FCEUD_GetTimeFreq(void);
bool Show_FPS = false;
// Control whether the frames per second of the emulation is rendered.
bool FCEUI_ShowFPS()
{
	return Show_FPS;
}
void FCEUI_SetShowFPS(bool showFPS)
{
	Show_FPS = showFPS;
}
void FCEUI_ToggleShowFPS()
{
	Show_FPS ^= 1;
}

static uint64 boop[60];
static int boopcount = 0;

void ShowFPS(void)
{
	if(Show_FPS == false)
		return;
	uint64 da = FCEUD_GetTime() - boop[boopcount];
	char fpsmsg[16];
	int booplimit = PAL?50:60;
	boop[boopcount] = FCEUD_GetTime();

	sprintf(fpsmsg, "%.1f", (double)booplimit / ((double)da / FCEUD_GetTimeFreq()));
	DrawTextTrans(XBuf + ((256 - ClipSidesOffset) - 40) + (FSettings.FirstSLine + 4) * 256, 256, (uint8*)fpsmsg, 0xA0);
	// It's not averaging FPS over exactly 1 second, but it's close enough.
	boopcount = (boopcount + 1) % booplimit;
}
