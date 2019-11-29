/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <klib.h>
#include "palette/palette.h"
#include "nes/nes.h"
#include "system/system.h"
#include "system/video.h"
#include "misc/memutil.h"
#include "misc/config.h"
#include "system/common/filters.h"

#define screenbpp 32

//system related variables
static int screenw,screenh;
static int screenscale;

//palette data fed to video system
static u32 palette32[8][256];		//32 bit color

//caches of all available colors
static u32 palettecache32[256];

//for frame limiting
static int interval = 0;
static u64 lasttime = 0;

//pointer to scree and copy of the nes screen
static uint32_t screen[256 * (240 + 16) * (screenbpp / 8) * 4] = {0};

//for correct colors
//static int rshift,gshift,bshift;
//static int rloss,gloss,bloss;
#define rshift 16
#define gshift 8
#define bshift 0

int video_init()
{
	//setup timer to limit frames
	interval = system_getfrequency() / 60;
	lasttime = system_gettick();

	//clear palette caches
	memset(palettecache32,0,256*sizeof(u32));

	//set screen info
	screenscale = config_get_int("video.scale");

	//initialize the video filters
	filter_init();

  screenw = screen_width();
  screenh = screen_height();

	return(0);
}

void video_kill()
{
	filter_kill();
}

int video_reinit()
{
	video_kill();
	return(video_init());
}

void video_startframe()
{
}

void video_endframe()
{
	//draw everything
  draw_rect(screen, 0, 0, 256, 240);
  draw_sync();

#ifdef __ISA_NATIVE__
	//simple frame limiter
	u64 t;
  do {
    t = system_gettick();
  } while(t - lasttime < interval);
  lasttime = t;
#endif
}

//this handles lines for gui/status messages
void video_updateline(int line,u8 *s)
{
	uint32_t *dest = screen + (line * 256);
	int i;

	if(line >= 8 && line < 232) {
		for(i=0;i<256;i++) {
			*dest++ = palettecache32[*s++];
		}
	}
	else {
		for(i=0;i<256;i++) {
			*dest++ = 0;
		}
	}
}

//this handles pixels coming directly from the nes engine
void video_updatepixel(int line,int pixel,u8 s)
{
	int offset = (line * 256) + pixel;

  screen[offset] = palettecache32[s];
}

//this handles palette changes from the nes engine
void video_updatepalette(u8 addr,u8 data)
{
	palettecache32[addr+0x00] = palette32[0][data];
	palettecache32[addr+0x20] = palette32[1][data];
	palettecache32[addr+0x40] = palette32[2][data];
	palettecache32[addr+0x60] = palette32[3][data];
	palettecache32[addr+0x80] = palette32[4][data];
	palettecache32[addr+0xA0] = palette32[5][data];
	palettecache32[addr+0xC0] = palette32[6][data];
	palettecache32[addr+0xE0] = palette32[7][data];
}

//must be called AFTER video_init
void video_setpalette(palette_t *p)
{
	int i,j;
	palentry_t e;

	for(j=0;j<8;j++) {
		for(i=0;i<256;i++) {
			e.val = p->pal[j][i & 0x3F];
			palette32[j][i] = (e.r << rshift) | (e.g << gshift) | (e.b << bshift);
		}
	}

	filter_palette_changed();
}

int video_getwidth()				{	return(screenw);			}
int video_getheight()			{	return(screenh);			}
int video_getbpp()				{	return(screenbpp);		}

//kludge-city!
int video_getxoffset()	{	return(0);	}
int video_getyoffset()	{	return(0);	}
int video_getscale()		{	return(screenscale);	}
